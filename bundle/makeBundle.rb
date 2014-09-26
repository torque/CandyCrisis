#!/usr/bin/env ruby

# In order to bundle libraries properly, the easiest method is, if the
# main binary is linked with an rpath during compilation to:

# Set all of the linked libraries in the executable to @rpath/library.dylib
# For dependencies of the linked libraries, set them to be @loader_path/library.dylib.

class OSXBundle

	SystemLibDirs   = /^(\/System\/Library|\/usr\/lib)/
	# https://developer.apple.com/library/mac/documentation/Darwin/Reference/ManPages/man1/dyld.1.html
	# The default DYLD_FALLBACK_LIBRARY_PATH is $(HOME)/lib:/usr/local/lib:/lib:/usr/lib.

	# If a linked library is not found by its install name it is searched
	# for in the paths in DYLD_FALLBACK_LIBRARY_PATH.
	DefaultLibPaths = ENV['DYLD_FALLBACK_LIBRARY_PATH']? ENV['DYLD_FALLBACK_LIBRARY_PATH'].split(':'): ["#{ENV['HOME']}/lib", "/usr/local/lib", "/lib", "/usr/lib"]
	LibraryName     = /\s*(.+?)[\(\s:]/
	InfoPlistToken  = /#\{.+?\}/

	def initialize( bundleName, contents )
		appName       = "#{bundleName}.app"
		if File.exists?( appName )
			`rm -rf "#{appName}"`
		end
		@baseAppDir   = "#{appName}/Contents"
		@infoPlist    = contents[:plist]
		@icon         = contents[:icon]
		@resources    = contents[:resources]
		@executables  = contents[:executables]

		@exeDirectory = "#{@baseAppDir}/MacOS"
		# Copy all dylibs to the Frameworks directory in the app bundle even
		# though they aren't actually frameworks.
		@libDirectory = "#{@baseAppDir}/Frameworks"
		@resourceDir  = "#{@baseAppDir}/Resources"
		# @masterLibList is a hash that contains all of the encountered
		# libraries, mapped to their fixed paths. It is used for copying all
		# necessary libraries into @libDirectory
		@masterLibList = {}
		# libtree is a hash that maps each non-system dependency to all of
		# its non-system dependencies.
		@libTree = {}
		# Wildcards to be replaced in the plist.
		@info = {
			'NAME' => bundleName,
			'ICON' => File.basename(@icon, File.extname(@icon)),
			'VERSION' => getVersion( )
		}
	end

	def getVersion
		version = `git describe --abbrev=10 --tags 2>&1`
		if version.match /^fatal: No names found/
			version = "git-" + `git rev-parse --short=10 @`
		end
		return version.chomp
	end

	def makeBundleSkeleton
		`mkdir -p "#{@exeDirectory}" "#{@libDirectory}"`
		@resources.each do |resource|
			`cp -R "#{resource}" "#{@resourceDir}"`
		end
	end

	# The icon goes in the bundle's resource directory. At least that's
	# where XCode puts it.
	def copyIcon
		`cp -R "#{@icon}" "#{@resourceDir}"`
	end

	def copyPlistWithTokens
		plistContents = nil
		File.open @infoPlist, 'r' do |plist|
			plistContents = plist.read
		end
		plistContents.gsub! InfoPlistToken do |match|
			@info[match[2..-2]]
		end
		File.open "#{@baseAppDir}/Info.plist", 'w' do |plist|
			plist.write plistContents
		end
	end

	def fixLib( lib )
		filename = File.basename lib
		errMessage = "Could not find library #{lib}. Searched the following paths:\n"
		DefaultLibPaths.each do |path|
			fullLibPath = "#{path}/#{filename}"
			errMessage += "-> #{fullLibPath}\n"
			if File.exist? fullLibPath
				return fullLibPath
			end
		end
		# Exit with an error here because a required library apparently does
		# not exist, which means the application can't be bundled correctly.
		print errMessage
		exit 1
	end

	def collectLibs( exe )
		# Need to fix exe name because collectLibs will be called with a
		# broken library name if one exists.
		linkedLibs = `otool -L #{File.exist?( exe )? exe: fixLib( exe )}`
		# Have to catch the possibility of passing in things that aren't
		# object files because one of our executables is just a shell
		# script.
		unless @libTree[exe] || linkedLibs.match( /is not an object file/ )
			@libTree[exe] = []
			lines = linkedLibs.split( /\n/ )
			lines.each do |lib|
				fixedLib = lib = lib.match( LibraryName )[1]
				unless File.exist? lib
					fixedLib = fixLib lib
				end
				# `otool -L` lists the library identification name at the top,
				# so we have to make sure we don't end up recursing infinitely.
				# The library basename should include version numbers and be
				# unique.
				unless lib.match( SystemLibDirs ) || File.basename( lib ) == File.basename( exe )
					# The library needs to be added to libTree even if it's been
					# seen before.
					@libTree[exe] << lib
					unless @masterLibList[lib]
						@masterLibList[lib] = fixedLib
					end
				end
			end
			@libTree[exe].each do |lib|
				collectLibs( lib )
			end
		end
	end

	def collectExe
		@executables.each do |exe|
			collectLibs( exe )
		end
	end

	def copyExe
		@executables.each do |exe|
			`cp "#{exe}" "#{@exeDirectory}"`
		end
	end

	# Expects copyExe to have been called first.
	def fixExeLinks
		@executables.each do |exe|
			exeName = File.basename( exe )
			if @libTree[exe] != nil
				@libTree[exe].each do |lib|
					libName = File.basename( lib )
					`install_name_tool -change "#{lib}" "@rpath/#{libName}" "#{@exeDirectory}/#{exeName}"`
				end
			end
		end
	end

	def copyLibs
		@masterLibList.each_value do |fixedLib|
			if fixedLib != true
				`cp "#{fixedLib}" "#{@libDirectory}"`
				# Fucking homebrew stores installed libraries as read only for
				# some reason that is beyond me.
				`chmod 755 "#{@libDirectory}/#{File.basename fixedLib}"`
			end
		end
	end

	# Expects copyLibs to have been called first.
	def fixLibLinks
		@masterLibList.each_key do |lib|
			libName = File.basename lib
			@libTree[lib].each do |childLib|
				childName = File.basename childLib
				`install_name_tool -change "#{childLib}" "@loader_path/#{childName}" "#{@libDirectory}/#{libName}"`
			end
		end
	end
end

CandyCrisis = OSXBundle.new( "Candy Crisis", {
	:plist       => "bundle/Info.plist",
	:icon        => "bundle/CandyCrisis.icns",
	:resources   => [ "CandyCrisisResources" ],
	:executables => [ "bundle/bundleShim.sh", "CandyCrisis" ]
})
CandyCrisis.makeBundleSkeleton
CandyCrisis.copyIcon
CandyCrisis.copyPlistWithTokens
CandyCrisis.collectExe
CandyCrisis.copyExe
CandyCrisis.fixExeLinks
CandyCrisis.copyLibs
CandyCrisis.fixLibLinks
