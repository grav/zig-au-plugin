const std = @import("std");
const build_options = @import("build_options");

var last: f32 = 0;

// Example Zig processing function
export fn processAudio(in_buffer: [*]const f32, out_buffer: [*]f32, frames: usize, volume: f32) void {
    if (build_options.enable_debug_logging) {
        std.debug.print("volume: {d}\n", .{volume});
    }
    var i: usize = 0;
    while (i < frames) : (i += 1) {
        out_buffer[i] = (last + in_buffer[i]) / 2 * volume;
        // out_buffer[i] = 0; //i % 4 == 0 ? in_buffer[i] : 0;
        last = in_buffer[i];
    }
}
