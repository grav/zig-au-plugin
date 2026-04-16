const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const root_module = b.createModule(.{
        .root_source_file = b.path("src/main.zig"),
        .target = target,
        .optimize = optimize,
    });

    root_module.addCSourceFile(.{
        .file = b.path("src/wrapper.c"),
        .flags = &.{"-fblocks"},
    });
    root_module.linkSystemLibrary("c", .{});
    root_module.linkFramework("AudioToolbox", .{});
    root_module.linkFramework("CoreFoundation", .{});

    const lib = b.addLibrary(.{
        .linkage = .dynamic,
        .name = "MyZigPlugin",
        .root_module = root_module,
    });

    b.installArtifact(lib);

    // Create the .component bundle
    const install_bin = b.addInstallArtifact(lib, .{
        .dest_dir = .{ .override = .{ .custom = "MyZigPlugin.component/Contents/MacOS" } },
    });
    b.getInstallStep().dependOn(&install_bin.step);

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
