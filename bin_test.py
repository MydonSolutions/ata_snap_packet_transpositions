import struct
import numpy as np

binary_data = None
with open("./dump.bin", "rb") as fio:
    binary_data = fio.read()

byte_size = len(binary_data)

byte_data = struct.unpack("B"*byte_size, binary_data)

# reshape to (time_major, nchan, nant, npol, complex, 4)
byte_shape = (64, 4, 2, 2, 4)
byte_shape = (byte_size//np.prod(byte_shape), *byte_shape)
byte_array = np.array(byte_data).reshape(byte_shape)

# shaped (time, nant, nchan, npol)
tafp = (byte_shape[0]*4, byte_shape[2], byte_shape[1], byte_shape[3])

for t in range(tafp[0]):
    for a in range(tafp[1]):
        for f in range(tafp[2]):
            for p in range(tafp[3]):
                if a*f != byte_array[t//4, f, a, p, 0, t%4]:
                    print("real! t=%d: [tM=%d, f=%d, a=%d, p=%d, i=%d, m=%d]: %d != %d" % (t, t//4, f, a, p, 0, t%4, byte_array[t//4, f, a, p, 0, t%4], a*f))
                if (t%128)*(p+1) != byte_array[t//4, f, a, p, 1, t%4]:
                    print("imag! t=%d: [tM=%d, f=%d, a=%d, p=%d, i=%d, m=%d]: %d != %d" % (t, t//4, f, a, p, 1, t%4, byte_array[t//4, f, a, p, 1, t%4], (t%128)*(p+1)))
