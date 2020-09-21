import logging
import queue
import sys
import traceback
import threading


class DecoderBase:
    def __init__(self, output_queue):
        self.output_queue_ = output_queue

    def process_line(self, line):
        print("process_line")
