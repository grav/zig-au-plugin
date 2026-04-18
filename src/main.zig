const std = @import("std");

var last: f32 = 0;

// Example Zig processing function
export fn processAudio(in_buffer: [*]const f32, out_buffer: [*]f32, frames: usize, volume: f32) void {
    var i: usize = 0;
    while (i < frames) : (i += 1) {
        out_buffer[i] = (last + in_buffer[i])/2 * volume;
        last = in_buffer[i];
    }
}
