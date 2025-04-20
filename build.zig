const std = @import("std");

const sources = &.{
    "src/main.c",
    "src/camera.c",
    "src/input.c",
    "src/meshes.c",
    "src/shaders.c",
};

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const root = b.createModule(.{
        .target = target,
        .optimize = optimize,
    });

    root.addCSourceFiles(.{
        .files = sources,
    });

    root.addIncludePath(b.path("src"));
    root.addIncludePath(b.path("deps/sdl/build/include"));
    root.addIncludePath(b.path("deps/sdl/build/include/SDL2"));
    root.addIncludePath(b.path("deps/sdl/build/include-config-debug/SDL2"));
    root.addIncludePath(b.path("deps/cglm/include"));
    root.addIncludePath(b.path("deps/glew/include"));

    root.addLibraryPath(b.path("deps/sdl/build"));
    root.addLibraryPath(b.path("deps/glew/lib"));

    root.linkSystemLibrary("SDL2maind", .{});
    root.linkSystemLibrary("SDL2d", .{});
    root.linkSystemLibrary("GLEW", .{});

    switch (target.result.os.tag) {
        .macos => {
            root.linkFramework("OpenGL", .{});
            root.linkFramework("Metal", .{});
            root.linkFramework("CoreVideo", .{});
            root.linkFramework("Cocoa", .{});
            root.linkFramework("IOKit", .{});
            root.linkFramework("ForceFeedback", .{});
            root.linkFramework("Carbon", .{});
            root.linkFramework("CoreAudio", .{});
            root.linkFramework("AudioToolbox", .{});
            root.linkFramework("AVFoundation", .{});
            root.linkFramework("Foundation", .{});
            root.linkFramework("GameController", .{});
            root.linkFramework("CoreHaptics", .{});
        },
        else => {},
    }

    const exe = b.addExecutable(.{
        .name = "cubes",
        .root_module = root,
    });

    b.installArtifact(exe);

    const run = b.step("run", "run cubes");
    const run_cmd = b.addRunArtifact(exe);
    run.dependOn(&run_cmd.step);
}
