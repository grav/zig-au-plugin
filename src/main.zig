const std = @import("std");

// Example Zig processing function
export fn processAudio(in_buffer: [*]const f32, out_buffer: [*]f32, frames: usize, volume: f32) void {
    std.debug.print("volume: {d}\n",.{volume});
    var i: usize = 0;
    while (i < frames) : (i += 1) {
        out_buffer[i] = in_buffer[i] * volume;
    }
}
