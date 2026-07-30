import struct

def write_bmp(filename, width, height, pixels_rgb):
    row_size = (width * 3 + 3) & ~3
    pixel_data_size = row_size * height
    file_size = 54 + pixel_data_size

    with open(filename, 'wb') as f:
        f.write(b'BM')
        f.write(struct.pack('<I', file_size))
        f.write(struct.pack('<H', 0))
        f.write(struct.pack('<H', 0))
        f.write(struct.pack('<I', 54))
        
        f.write(struct.pack('<I', 40))
        f.write(struct.pack('<i', width))
        f.write(struct.pack('<i', -height))
        f.write(struct.pack('<H', 1))
        f.write(struct.pack('<H', 24))
        f.write(struct.pack('<I', 0))
        f.write(struct.pack('<I', pixel_data_size))
        f.write(struct.pack('<i', 2835))
        f.write(struct.pack('<i', 2835))
        f.write(struct.pack('<I', 0))
        f.write(struct.pack('<I', 0))
        
        for y in range(height):
            for x in range(width):
                r, g, b = pixels_rgb[y * width + x]
                f.write(struct.pack('<BBB', b, g, r))
            for _ in range(row_size - width * 3):
                f.write(b'\x00')

width = 640
height = 400

mat = [(0, 0, 0)] * (width * height)
alb = [(0, 0, 0)] * (width * height)

for y in range(height):
    for x in range(width):
        idx = y * width + x
        if y > 300: 
            mat[idx] = (0x88, 0x88, 0x88) # Wall
            alb[idx] = (0x77, 0x77, 0x77)
            if 300 < x < 400 and y < 350:
                mat[idx] = (0x44, 0x44, 0xFF) # Water
                alb[idx] = (0x33, 0x33, 0xEE)
        elif y > 200 and x < 150:
            mat[idx] = (0xEE, 0xDD, 0x82) # Sand (powder piling)
            alb[idx] = (0xEE, 0xDD, 0x82)
        elif y > 250 and 450 < x < 470:
            mat[idx] = (0x6B, 0x44, 0x23) # Wood (structural collapse)
            alb[idx] = (0x55, 0x33, 0x11)
        else:
            mat[idx] = (0, 0, 0)
            alb[idx] = (0, 0, 0)

write_bmp('assets/test_material.bmp', width, height, mat)
write_bmp('assets/test_albedo.bmp', width, height, alb)
