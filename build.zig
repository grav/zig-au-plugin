const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    // 1. Build the Zig DSP logic as a separate dynamic library (libdsp.dylib)
    const dsp_module = b.createModule(.{
        .root_source_file = b.path("src/main.zig"),
        .target = target,
        .optimize = optimize,
    });
    const dsp_lib = b.addLibrary(.{
        .linkage = .dynamic,
        .name = "dsp",
        .root_module = dsp_module,
    });

    // Install the DSP library into the .component bundle's MacOS folder alongside the wrapper
    const install_dsp = b.addInstallArtifact(dsp_lib, .{
        .dest_dir = .{ .override = .{ .custom = "MyZigPlugin.component/Contents/MacOS" } },
    });
    b.getInstallStep().dependOn(&install_dsp.step);

    // 2. Build the C wrapper as the main plugin binary (libMyZigPlugin.dylib)
    const wrapper_module = b.createModule(.{
        .target = target,
        .optimize = optimize,
    });
    wrapper_module.addCSourceFile(.{
        .file = b.path("src/wrapper.c"),
        .flags = &.{"-fblocks"},
    });
    wrapper_module.linkSystemLibrary("c", .{});
    wrapper_module.linkFramework("AudioToolbox", .{});
    wrapper_module.linkFramework("CoreFoundation", .{});
    wrapper_module.linkFramework("Cocoa", .{});

    const wrapper_lib = b.addLibrary(.{
        .linkage = .dynamic,
        .name = "MyZigPlugin",
        .root_module = wrapper_module,
    });

    // Install the wrapper into the .component bundle
    const install_wrapper = b.addInstallArtifact(wrapper_lib, .{
        .dest_dir = .{ .override = .{ .custom = "MyZigPlugin.component/Contents/MacOS" } },
    });
    b.getInstallStep().dependOn(&install_wrapper.step);

    const info_plist =
        \\<?xml version="1.0" encoding="UTF-8"?>
        \\<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
        \\<plist version="1.0">
        \\<dict>
        \\    <key>CFBundleDevelopmentRegion</key>
        \\    <string>English</string>
        \\    <key>CFBundleExecutable</key>
        \\    <string>libMyZigPlugin.dylib</string>
        \\    <key>CFBundleIdentifier</key>
        \\    <string>com.example.zigplugin</string>
        \\    <key>CFBundleInfoDictionaryVersion</key>
        \\    <string>6.0</string>
        \\    <key>CFBundleName</key>
        \\    <string>MyZigPlugin</string>
        \\    <key>CFBundlePackageType</key>
        \\    <string>BNDL</string>
        \\    <key>CFBundleShortVersionString</key>
        \\    <string>1.0</string>
        \\    <key>CFBundleSignature</key>
        \\    <string>????</string>
        \\    <key>CFBundleVersion</key>
        \\    <string>1.0</string>
        \\    <key>AudioComponents</key>
        \\    <array>
        \\        <dict>
        \\            <key>description</key>
        \\            <string>Zig Audio Plugin</string>
        \\            <key>factoryFunction</key>
        \\            <string>MyZigPluginFactory</string>
        \\            <key>manufacturer</key>
        \\            <string>Zigg</string>
        \\            <key>name</key>
        \\            <string>Demo: ZigPlugin</string>
        \\            <key>subtype</key>
        \\            <string>volu</string>
        \\            <key>type</key>
        \\            <string>aufx</string>
        \\            <key>version</key>
        \\            <integer>65536</integer>
        \\        </dict>
        \\    </array>
        \\</dict>
        \\</plist>
    ;

    const wf = b.addWriteFiles();
    const plist_file = wf.add("Info.plist", info_plist);

    const install_plist = b.addInstallFile(plist_file, "MyZigPlugin.component/Contents/Info.plist");
    b.getInstallStep().dependOn(&install_plist.step);
}
