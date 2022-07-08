const std = @import("std");

pub fn build(b: *std.build.Builder) void {
    // Standard target options allows the person running `zig build` to choose
    // what target to build for. Here we do not override the defaults, which
    // means any target is allowed, and the default is native. Other options
    // for restricting supported target set are available.
    const target = b.standardTargetOptions(.{});

    // Standard release options allow the person running `zig build` to select
    // between Debug, ReleaseSafe, ReleaseFast, and ReleaseSmall.
    const mode = b.standardReleaseOptions();

    const exe = b.addExecutable("CandyCrisis", "src/options.zig");
    exe.addIncludeDir("src");
    exe.setTarget(target);
    exe.setBuildMode(mode);
    exe.install();

    const testexe = b.addExecutable("sdltest", "test/test.zig");
    testexe.linkLibC();
    testexe.setTarget(target);
    testexe.setBuildMode(mode);
    testexe.linkSystemLibrary("sdl2");
    testexe.linkSystemLibrary("sdl2_image");
    testexe.linkSystemLibrary("libjpeg");
    testexe.linkSystemLibrary("libpng");
    testexe.linkSystemLibrary("libwebp");
    testexe.linkSystemLibrary("libtiff-4");
    testexe.linkSystemLibraryName("iconv");
    testexe.linkFramework("IOKit");
    testexe.linkFramework("Cocoa");
    testexe.linkFramework("CoreAudio");
    testexe.linkFramework("Carbon");
    testexe.linkFramework("Metal");
    testexe.linkFramework("QuartzCore");
    testexe.linkFramework("AudioToolbox");
    testexe.linkFramework("ForceFeedback");
    testexe.linkFramework("GameController");
    testexe.linkFramework("CoreHaptics");

    const install_test = b.addInstallArtifact(testexe);

    const run_test = testexe.run();
    run_test.step.dependOn(&install_test.step);
    const rts = b.step("st", "sdl test");
    rts.dependOn(&run_test.step);

    const run_cmd = exe.run();
    run_cmd.step.dependOn(b.getInstallStep());
    if (b.args) |args| {
        run_cmd.addArgs(args);
    }

    const run_step = b.step("run", "Run the app");
    run_step.dependOn(&run_cmd.step);

    const exe_tests = b.addTest("src/options.zig");
    exe_tests.setTarget(target);
    exe_tests.setBuildMode(mode);

    const test_step = b.step("test", "Run unit tests");
    test_step.dependOn(&exe_tests.step);
}
