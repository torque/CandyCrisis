const std = @import("std");

const Brand = enum {
    Option,
    Flag,
    Argument,
};

pub fn noopHandleGen(comptime intype: type) fn (buf: []const u8) anyerror!intype {
    return struct {
        pub fn handler(input: []const u8) anyerror!intype {
            return input;
        }
    }.handler;
}

const noopHandler = noopHandleGen([]const u8);
const noOptHandler = noopHandleGen(?[]const u8);

pub fn intHandler(comptime intType: type, radix: u8) fn (buf: []const u8) std.fmt.ParseIntError!?intType {
    return struct {
        pub fn handleInt(buf: []const u8) std.fmt.ParseIntError!?intType {
            return try std.fmt.parseInt(intType, buf, radix);
        }
    }.handleInt;
}

const i32Handler = intHandler(i32, 0);

pub const OptionError = error{
    BadShortOption,
    BadLongOption,
    UnknownOption,
    MissingOption,
    MissingArgument,
    ExtraArguments,
};

pub const ArgCountCategory = enum {
    None,
    Some,
    Many,
};

pub const ArgCount = union(ArgCountCategory) {
    None: void,
    Some: u32,
    Many: void,
};

pub fn ValuedOption(comptime resultType: type) type {
    return struct {
        pub const brand: Brand = .Option;

        name: []const u8,
        // compiler bug?????
        default: union(enum) { none: void, value: resultType } = .none,
        // this is a combination conversion/validation callback.
        // Should we try to pass a user context? Zig's bound functions
        // don't seem to coerce nicely to this type, probably because
        // they're no longer just a pointer.
        handler: fn (input: []const u8) anyerror!resultType = noopHandler,
        short: ?[]const u8 = null,
        long: ?[]const u8 = null,
        help: ?[]const u8 = null,
        envVar: ?[]const u8 = null,
        hideResult: bool = false,
        eager: bool = false,

        pub fn ResultType(_: @This()) type {
            return resultType;
        }

        pub fn optional(self: @This()) bool {
            return self.default == .none;
        }
    };
}

pub const StringOption = ValuedOption([]const u8);

// this could be ValuedOption(bool) except it allows truthy/falsy flag variants
// and it doesn't want to parse a value. It could be lowered into a pair of
// ValuedOption(bool) though, if consuming a value became optional.

const TruthyFalsy = struct {
    truthy: ?[]const u8 = null,
    falsy: ?[]const u8 = null,
};

pub const FlagOption = struct {
    pub const brand: Brand = .Flag;

    name: []const u8,
    default: union(enum) { none: void, value: bool } = .{ .value = false },
    short: TruthyFalsy = .{},
    long: TruthyFalsy = .{},
    help: ?[]const u8 = null,
    // should envVar be split into truthy/falsy the way the args are? otherwise
    // we probably need to peek the value of the environmental variable to see
    // if it is truthy or falsy. Honestly, looking at the value is probably
    // required to avoid violating the principle of least astonishment because
    // otherwise you can get `MY_VAR=false` causing `true` to be emitted, which
    // looks and feels bad. But then we need to establish a truthiness baseline.
    // case insensitive true/false is easy. What about yes/no? 0/1 (or nonzero).
    // How about empty strings? I'd base on how it reads, and `MY_VAR= prog`
    // reads falsy to me.
    envVar: ?[]const u8 = null,
    hideResult: bool = false,
    eager: bool = false,

    pub fn ResultType(_: @This()) type {
        return bool;
    }
};

const HelpFlag = FlagOption{
    .name = "help",
    .short = .{ .truthy = "-h" },
    .long = .{ .truthy = "--help" },
    .help = "print this help message",
    .hideResult = true,
    .eager = true,
};

pub fn Argument(comptime resultType: type) type {
    return struct {
        pub const brand: Brand = .Argument;

        name: []const u8,
        default: union(enum) { none: void, value: resultType } = .none,
        handler: fn (input: []const u8) anyerror!resultType = noopHandler,
        help: ?[]const u8 = null,
        hideResult: bool = false,
        // allow loading arguments from environmental variables? I don't think it's possible to come up with sane semantics for this.

        pub fn ResultType(_: @This()) type {
            return resultType;
        }
    };
}

const StringArg = Argument([]const u8);

/// spec is a tuple of ValuedOption, FlagOption, and Argument
pub fn Command(comptime spec: anytype) type {
    comptime var argCount = 0;
    comptime var requiredOptions = 0;
    comptime for (spec) |param| {
        switch (@TypeOf(param).brand) {
            .Argument => argCount += 1,
            .Option, .Flag => if (param.default == .none) {
                requiredOptions += 1;
            },
        }
    };

    return struct {
        name: []const u8,
        help: ?[]const u8 = null,

        pub fn formatHelp(self: @This()) []const u8 {
            return self.help orelse "";
        }

        pub fn parse(_: @This(), comptime argit_type: type, argit: *argit_type) !CommandResult(spec) {
            // @compileLog("parse");
            _ = argit.next();

            // we can't actually make use of the default values because there's
            // no way to only partially initialize the struct without a big
            // goofy hack (maybe I will do that later)
            var result: CommandResult(spec) = undefined;

            inline for (spec) |param| {
                if (param.hideResult == false) {
                    @field(result, param.name) = switch (param.default) {
                        .none => continue,
                        .value => |val| val,
                    };
                }
            }

            var seenArgs: u32 = 0;
            var seenReqdOpts: u32 = 0;
            argloop: while (argit.next()) |arg| {
                // is arg.len > 0 a redundant check? I guess not
                if (arg.len > 1 and arg[0] == '-') {
                    if (arg.len == 2 and arg[1] == '-') continue :argloop;

                    if (arg[1] == '-') {
                        // we have a long flag or option
                        specloop: inline for (spec) |param| {
                            switch (@TypeOf(param).brand) {
                                .Option => {
                                    // have to force lower the handler to runtime
                                    var handler = param.handler;
                                    if (param.long) |flag| {
                                        if (std.mem.eql(u8, flag, arg)) {
                                            if (param.default == .none) seenReqdOpts += 1;

                                            const val = argit.next() orelse break :argloop;
                                            if (param.hideResult == false) {
                                                @field(result, param.name) = try handler(val);
                                            }
                                            continue :argloop;
                                        }
                                    }
                                },
                                .Flag => {
                                    if (param.long.truthy) |truthy| {
                                        if (std.mem.eql(u8, truthy, arg)) {
                                            if (param.default == .none) seenReqdOpts += 1;

                                            if (param.hideResult == false) {
                                                @field(result, param.name) = true;
                                            }
                                            continue :argloop;
                                        }
                                    }
                                    if (param.long.falsy) |falsy| {
                                        if (std.mem.eql(u8, falsy, arg)) {
                                            if (param.default == .none) seenReqdOpts += 1;

                                            if (param.hideResult == false) {
                                                @field(result, param.name) = false;
                                            }
                                            continue :argloop;
                                        }
                                    }
                                },
                                .Argument => continue :specloop,
                            }
                        }

                        // nothing matched
                        return OptionError.UnknownOption;
                    } else {
                        // we have a short flag, which may be multiple fused flags
                        shortloop: for (arg[1..]) |shorty, idx| {
                            std.debug.print("check {c}\n", .{shorty});
                            specloop: inline for (spec) |param| {
                                switch (@TypeOf(param).brand) {
                                    .Option => {
                                        var handler = param.handler;
                                        if (param.short) |flag| {
                                            if (flag[1] == shorty) {
                                                if (param.default == .none) seenReqdOpts += 1;

                                                const val = if (arg.len > (idx + 2)) arg[(idx + 2)..] else argit.next() orelse return OptionError.MissingArgument;
                                                if (param.hideResult == false) {
                                                    @field(result, param.name) = try handler(val);
                                                }
                                                continue :argloop;
                                            }
                                        }
                                    },
                                    .Flag => {
                                        if (param.short.truthy) |truthy| {
                                            if (truthy[1] == shorty) {
                                                if (param.default == .none) seenReqdOpts += 1;

                                                if (param.hideResult == false) {
                                                    @field(result, param.name) = true;
                                                }
                                                continue :shortloop;
                                            }
                                        }
                                        if (param.short.falsy) |falsy| {
                                            if (falsy[1] == shorty) {
                                                if (param.default == .none) seenReqdOpts += 1;

                                                if (param.hideResult == false) {
                                                    @field(result, param.name) = false;
                                                }
                                                continue :shortloop;
                                            }
                                        }
                                    },
                                    .Argument => continue :specloop,
                                }
                            }
                            // nothing matched
                            return OptionError.UnknownOption;
                        }
                    }
                } else {
                    // we have a subcommand or an Argument.
                    comptime var idx = 0;
                    specloop: inline for (spec) |param| {
                        switch (@TypeOf(param).brand) {
                            .Argument => {
                                var handler = param.handler;

                                std.debug.print("{s}, idx: {}, seenArgs: {}\n", .{ arg, idx, seenArgs });

                                if (seenArgs == idx) {
                                    @field(result, param.name) = try handler(arg);
                                    seenArgs += 1;
                                }
                                idx += 1;
                            },
                            else => continue :specloop,
                        }
                    }
                }
            }

            if (seenArgs < argCount) {
                return OptionError.MissingArgument;
            } else if (seenArgs > argCount) {
                return OptionError.ExtraArguments;
            } else if (seenReqdOpts != requiredOptions) {
                return OptionError.MissingOption;
            }

            return result;
        }
    };
}

pub fn CommandResult(comptime spec: anytype) type {
    const StructField = std.builtin.Type.StructField;

    comptime {
        // not sure how to do this without iterating twice, so let's iterate
        // twice

        var outsize = 0;
        for (spec) |param| {
            if (param.hideResult == false) outsize += 1;
        }

        var fields: [outsize]StructField = undefined;

        var idx = 0;
        for (spec) |param| {
            if (param.hideResult == true) continue;

            const fieldType = param.ResultType();

            fields[idx] = .{
                .name = param.name,
                .field_type = fieldType,
                .default_value = switch (param.default) {
                    .none => null,
                    .value => |val| &val,
                },
                .is_comptime = false,
                .alignment = @alignOf(fieldType),
            };

            idx += 1;
        }

        return @Type(.{ .Struct = .{
            .layout = .Auto,
            .fields = &fields,
            .decls = &.{},
            .is_tuple = false,
        } });
    }
}

pub fn main() !void {
    const command = Command(
        .{
            HelpFlag,
            // FlagOption(){
            //     .name = "flag",
            //     .default = .{ .value = false },
            //     .short = .{ .truthy = "-f" },
            //     .long = .{ .truthy = "--flag", .falsy = "--no-flag" },
            // },
            // StringOption{ .name = "input", .short = "-i", .long = "--input", .default = .{ .value = "waoh" } },
            // StringOption{ .name = "output", .long = "--output" },
            // ValuedOption(?i32){
            //     .name = "number",
            //     .short = "-n",
            //     .long = "--number",
            //     .default = .{ .value = null },
            //     .handler = i32Handler,
            // },
            // StringArg{ .name = "theman" },
        },
    ){ .name = "main", .help = 
    \\ Test command line functionality
    \\
    \\ This tests the command line argument parsing
    };

    var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
    defer arena.deinit();

    const allocator = arena.allocator();
    var argit = try std.process.argsWithAllocator(allocator);

    const result = try command.parse(std.process.ArgIterator, &argit);
    std.debug.print("res: {any}\n", .{result});
}
