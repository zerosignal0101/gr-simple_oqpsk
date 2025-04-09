import numpy as np
from gnuradio import gr
import pmt

class payloadExtractor(gr.sync_block):
    def __init__(self, debug=True):
        gr.sync_block.__init__(
            self,
            name="payloadExtractor",
            in_sig=[np.uint8],
            out_sig=None
        )
        self.access_code = [0xac, 0xdd, 0xa4, 0xe2, 0xf2, 0x8c, 0x20, 0xfc]
        self.access_code_bits = self.bytes_to_bits(self.access_code)
        self.state = "SEARCHING"
        self.buffer = []
        self.expected_length = 0
        self.bit_counter = 0
        self.debug = debug
        self.message_port_register_out(pmt.intern("pdu"))
        
        # Track how many bits we've processed in current state
        self.consumed = 0
        
        if self.debug:
            print("Initialized Payload Extractor with proper stream handling")

    def bytes_to_bits(self, byte_list):
        """Convert list of bytes to list of bits (MSB first)"""
        bits = []
        for byte in byte_list:
            for i in range(8):
                bits.append((byte >> (7 - i)) & 0x01)
        return bits
        
    def bits_to_bytes(self, bit_list):
        """Convert list of bits to bytes (MSB first)"""
        bytes_out = []
        for i in range(0, len(bit_list), 8):
            byte = 0
            for j in range(8):
                if i+j < len(bit_list):
                    byte |= (bit_list[i+j] << (7 - j))
            bytes_out.append(byte)
        return bytes_out
        
    def print_debug(self, message):
        if self.debug:
            print(f"[DEBUG] {message}")

    def work(self, input_items, output_items):
        in_bits = input_items[0]
        ninput = len(in_bits)
        self.print_debug(f"Entering work() with {ninput} bits, state: {self.state}")
        
        # Reset consumed counter at start of each work()
        self.consumed = 0
        
        while self.consumed < ninput:
            if self.state == "SEARCHING":
                tags = self.get_tags_in_window(0, self.consumed, ninput)
                found_tag = False
                
                for tag in tags:
                    if pmt.to_python(tag.key) == "header_tag":
                        tag_pos = tag.offset - self.nitems_read(0)
                        if tag_pos >= self.consumed:
                            # Consume bits up to the tag
                            self.consumed = tag_pos
                            self.state = "LENGTH"
                            self.buffer = []
                            found_tag = True
                            self.print_debug(f"Found header_tag at pos {tag_pos}, moving to ACCESS_CODE")
                            break
                
                if not found_tag:
                    self.print_debug(f"SEARCHING: No tag found, consuming all {ninput} bits")
                    self.consumed = ninput
                    return ninput
                    
            elif self.state == "ACCESS_CODE":
                needed = 64 - len(self.buffer)
                available = ninput - self.consumed
                take = min(needed, available)
                
                self.buffer.extend(in_bits[self.consumed:self.consumed+take])
                self.consumed += take
                
                if len(self.buffer) == 64:
                    received_ac = self.buffer[:64]
                    ac_bytes = self.bits_to_bytes(received_ac)
                    self.print_debug(f"Received Access Code Bytes: {[hex(x) for x in ac_bytes]}")
                    if received_ac == self.access_code_bits:
                        self.print_debug("ACCESS_CODE matched, moving to LENGTH")
                        self.state = "LENGTH"
                        self.buffer = []
                    else:
                        self.print_debug("ACCESS_CODE mismatch, returning to SEARCHING")
                        self.state = "SEARCHING"
                        
            elif self.state == "LENGTH":
                needed = 32 - len(self.buffer)  # 16 bits length + 16 bits duplicate
                available = ninput - self.consumed
                take = min(needed, available)
                
                self.buffer.extend(in_bits[self.consumed:self.consumed+take])
                self.consumed += take
                
                if len(self.buffer) == 32:
                    # Calculate length1 from first 16 bits (indices 0-15)
                    length1 = 0
                    self.print_debug("Calculating length1 from first 16 bits:")
                    for i, bit in enumerate(self.buffer[:16]):
                        shift_amount = 15 - i
                        shifted_value = int(bit) << shift_amount
                        length1 += shifted_value
                        self.print_debug(f"  Bit {i}: value={bit}, shift={shift_amount}, shifted={shifted_value:#06x}, accum={length1:#06x}")

                    self.print_debug(f"Final length1: {length1} (0x{length1:04x})\n")

                    # Calculate length2 from next 16 bits (indices 16-31)
                    length2 = 0
                    self.print_debug("Calculating length2 from next 16 bits:")
                    for i, bit in enumerate(self.buffer[16:32]):
                        shift_amount = 15 - i
                        shifted_value = int(bit) << shift_amount
                        length2 += shifted_value
                        self.print_debug(f"  Bit {i+16}: value={bit}, shift={shift_amount}, shifted={shifted_value:#06x}, accum={length2:#06x}")

                    self.print_debug(f"Final length2: {length2} (0x{length2:04x})")
                    
                    if length1 == length2:
                        self.expected_length = length1 * 8
                        self.print_debug(f"LENGTH matched: {length1} bytes, moving to PAYLOAD")
                        self.state = "PAYLOAD"
                        self.buffer = []
                    else:
                        self.print_debug(f"LENGTH mismatch: {length1} vs {length2}, returning to SEARCHING")
                        self.state = "SEARCHING"
                        
            elif self.state == "PAYLOAD":
                needed = self.expected_length - len(self.buffer)
                available = ninput - self.consumed
                take = min(needed, available)
                
                self.buffer.extend(in_bits[self.consumed:self.consumed+take])
                self.consumed += take
                
                if len(self.buffer) == self.expected_length:
                    payload_bytes = self.bits_to_bytes(self.buffer)
                    pdu = pmt.init_u8vector(len(payload_bytes), payload_bytes)
                    self.message_port_pub(pmt.intern("pdu"), pmt.cons(pmt.PMT_NIL, pdu))
                    self.print_debug(f"PAYLOAD complete, sent {len(payload_bytes)} bytes, returning to SEARCHING")
                    self.state = "SEARCHING"
                    self.buffer = []
        
        self.print_debug(f"Exiting work(), consumed {self.consumed} bits")
        return self.consumed