# Format

The binary format stores a fixed header followed by a section table. Each section record includes kind, flags, offset, stored size, and uncompressed size. Objects refer to one another by stable IDs rather than raw pointers.
