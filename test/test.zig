const std = @import("std");
const SDL = @cImport({
    @cInclude("SDL.h");
    @cInclude("SDL_Image.h");
});

const Renderer = SDL.SDL_Renderer;
const Texture = SDL.SDL_Texture;
const Point = SDL.SDL_Point;
const Rect = SDL.SDL_Rect;

const TestError = error{
    SdlInitFailure,
    SdlImgInitFailure,
    SdlWindowCreateFailure,
    SdlRendererCreateFailure,
    LoadTextureFailure,
};

const Option = struct {
    short: ?[]const u8
    long: ?[]const u8
    help: ?[]const u8
    type: type
}

fn Parser(comptime enumeration: type) type {
    return struct {
        valueType: enumeration

    }
}

fn loadTexture(renderer: *Renderer, filename: [:0]const u8) TestError!*Texture {
    return SDL.IMG_LoadTexture(renderer, filename) orelse TestError.LoadTextureFailure;
}

fn drawTexture(renderer: *Renderer, texture: *Texture, point: Point) void {
    var wid: c_int = 0;
    var hei: c_int = 0;
    if (SDL.SDL_QueryTexture(texture, null, null, &wid, &hei) != 0) {}
    const dest = Rect{ .x = point.x, .y = point.y, .w = @truncate(i16, wid), .h = @truncate(i16, hei) };
    if (SDL.SDL_RenderCopy(renderer, texture, null, &dest) != 0) {}
}

pub fn main() !void {
    if (SDL.SDL_Init(SDL.SDL_INIT_VIDEO) < 0) {
        std.debug.print("Couldn't initialize SDL: {s}\n", .{SDL.SDL_GetError()});
        return TestError.SdlInitFailure;
    }

    if (SDL.IMG_Init(SDL.IMG_INIT_PNG | SDL.IMG_INIT_JPG) == 0) {
        std.debug.print("sdl_image not working here", .{});
        return TestError.SdlImgInitFailure;
    }

    const winWidth = 640;
    const winHeight = 480;

    var window = SDL.SDL_CreateWindow(
        "sdltest",
        SDL.SDL_WINDOWPOS_CENTERED,
        SDL.SDL_WINDOWPOS_CENTERED,
        winWidth,
        winHeight,
        0,
    ) orelse {
        std.debug.print("Failed to open {d} x {d} window: {s}\n", .{ winWidth, winHeight, SDL.SDL_GetError() });
        return TestError.SdlWindowCreateFailure;
    };

    if (SDL.SDL_SetHint(SDL.SDL_HINT_RENDER_SCALE_QUALITY, "nearest") == SDL.SDL_FALSE) {
        std.debug.print("failed to set render scale quality hint\n", .{});
    }

    var renderer = SDL.SDL_CreateRenderer(
        window,
        -1,
        SDL.SDL_RENDERER_ACCELERATED | SDL.SDL_RENDERER_PRESENTVSYNC,
    ) orelse {
        std.debug.print("Failed to create renderer: {s}\n", .{SDL.SDL_GetError()});
        return TestError.SdlRendererCreateFailure;
    };

    var texture: *Texture = try loadTexture(renderer, "resources/blob.png");
    var textur2: *Texture = try loadTexture(renderer, "resources/eyes.png");

    if (SDL.SDL_SetRenderDrawColor(renderer, 96, 128, 255, 255) != 0) {
        std.debug.print("failed to set render draw color\n", .{});
    }
    if (SDL.SDL_RenderClear(renderer) != 0) {
        std.debug.print("failed to call render clear\n", .{});
    }
    drawTexture(renderer, texture, .{ .x = 0, .y = 0 });
    var y2: c_int = 0;
    if (SDL.SDL_QueryTexture(texture, null, null, null, &y2) != 0) {}
    drawTexture(renderer, textur2, .{ .x = 0, .y = y2 });
    SDL.SDL_RenderPresent(renderer);

    main: while (true) {
        var event: SDL.SDL_Event = undefined;
        while (SDL.SDL_PollEvent(&event) != 0) {
            switch (event.type) {
                SDL.SDL_QUIT => {
                    break :main;
                },
                else => {},
            }
        }
        if (SDL.SDL_RenderClear(renderer) != 0) {
            std.debug.print("failed to call render clear\n", .{});
        }
        drawTexture(renderer, texture, .{ .x = 0, .y = 0 });
        drawTexture(renderer, textur2, .{ .x = 0, .y = y2 });
        SDL.SDL_RenderPresent(renderer);

        // SDL.SDL_Delay(16);
    }
}
