import numpy as np
from gnuradio import gr

class CRCErrorCounter(gr.sync_block):
    def __init__(self):
        gr.sync_block.__init__(
            self,
            name="CRC Error Counter",
            in_sig=None,
            out_sig=None
        )
        self.ok_count = 0
        self.fail_count = 0
        
        self.message_port_register_in(gr.pmt.intern("ok"))
        self.message_port_register_in(gr.pmt.intern("fail"))
        self.set_msg_handler(gr.pmt.intern("ok"), self.handle_ok)
        self.set_msg_handler(gr.pmt.intern("fail"), self.handle_fail)
        
    def handle_ok(self, msg):
        self.ok_count += 1
        self.print_stats()
        
    def handle_fail(self, msg):
        self.fail_count += 1
        self.print_stats()
        
    def print_stats(self):
        total = self.ok_count + self.fail_count
        if total > 0:
            error_rate = (self.fail_count / total) * 100
            print(f"CRC Stats: OK={self.ok_count}, Fail={self.fail_count}, Error Rate={error_rate:.2f}%")