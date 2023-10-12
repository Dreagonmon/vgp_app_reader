from typing import Dict, Tuple
import zlib
import io

ASCII_DATA_START = (0X21)
ASCII_DATA_END = (0x7E)
ASCII_T = (9)
ASCII_N = (10)
ASCII_R = (13)
ASCII_SPACE = (32)
BASE_OFFSET = (770)# 2 + (256 * 3)

FONT8_FILE = "pix8x8.ufnt"
FONT16_FILE = "pix16x16.ufnt"
FONT8_ASCII_WIDTH = b'\x02\x04\x06\x06\x06\x06\x02\x03\x03\x06\x06\x03\x06\x03\x04\x05\x04\x05\x05\x05\x05\x05\x05\x05\x05\x03\x03\x05\x06\x05\x05\x08\x05\x05\x05\x05\x04\x04\x05\x05\x02\x04\x05\x04\x06\x05\x05\x05\x05\x05\x05\x06\x05\x06\x06\x06\x06\x05\x03\x04\x03\x04\x05\x03\x05\x05\x05\x05\x05\x04\x05\x05\x02\x04\x05\x03\x06\x05\x05\x05\x05\x04\x05\x04\x05\x06\x06\x06\x05\x05\x04\x02\x04\x07'
FONT16_ASCII_WIDTH = b'\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08'

class UnicodeBitmapFont():
    def __init__(self, font_stream, ascii_width=b''):
        self.__font_file = font_stream
        self.__seek = self.__font_file.seek
        self.__read = self.__font_file.read
        self.__area_offset = []
        self.__area_size = bytearray()
        self.__ascii_width = ascii_width # type: bytes
        self.__ascii_width_limit = len(ascii_width)
        self.__font_width = self.__font_file.read(1)[0]
        self.__font_height = self.__font_file.read(1)[0]
        w_block = self.__font_width // 8
        w_block += 0 if self.__font_width % 8 == 0 else 1
        self.__font_data_size = w_block * self.__font_height
        for i in range(256):
            offset = int.from_bytes(self.__font_file.read(2), 'big')
            # offset = self.__font_file.read(2)
            size = int.from_bytes(self.__font_file.read(1), 'big')
            if i == 0:
                # self.__area_offset.extend(b'\x00\x00')
                self.__area_offset.append(0)
            else:
                self.__area_offset.append(offset)
            self.__area_size.append(size)

    # @timed_function
    def get_char_data(self, unicode:int):
        if unicode > 0xFFFF:
            return
        area:int = unicode & 0xFF
        pos:int = (unicode & 0xFF00) >> 8
        # query char data
        seek = self.__seek
        read = self.__read
        font_data_size:int = int(self.__font_data_size)
        area_offset:int = int(self.__area_offset[area])
        area_size:int = int(self.__area_size[area])
        offset:int = int(area_offset * (font_data_size + 1)) + BASE_OFFSET
        seek(offset)
        pos_lst = read(area_size)
        data_not_found = True
        data = b''
        for i in range(area_size):
            if pos_lst[i] == pos:
                data_index = i
                offset += area_size + font_data_size * data_index
                seek(offset)
                data_not_found = False
                data = read(font_data_size)
                break
        if data_not_found:
            # not found
            return bytes(self.__font_data_size)
        return data

    def get_char_width(self, unicode: int) -> int:
        if unicode == ASCII_SPACE:
            return self.__font_width // 2 # half width space
        elif unicode == ASCII_N or unicode == ASCII_R:
            return 0
        ascii_offset = unicode - ASCII_DATA_START
        if ascii_offset >= 0 and ascii_offset < self.__ascii_width_limit:
            return self.__ascii_width[ascii_offset]
        else:
            return self.__font_width

    def get_font_size(self):
        return (self.__font_width, self.__font_height)

class UTF8():
    @staticmethod
    def byte_size(first_byte):
        pat = 0x01 << 7  # 1000000
        l = 0  # length
        while first_byte & pat != 0:
            l = l + 1
            pat = pat >> 1
        if l == 0:
            l = 1  # for ascii
        return l

    @staticmethod
    def unicode_byte_size(value):
        if value <= 0x7F:
            return 1 # ascii
        elif value <= 0x7FF:
            return 2 # 110 xxxxx
        elif value <= 0xFFFF:
            return 3 # 1110 xxxx
        elif value <= 0x1FFFF:
            return 4 # 11110 xxx
        elif value <= 0x7FFFFFF:
            return 5 # 111110 xx
        elif value <= 0x80000000:
            return 6 # 1111110 x
            # all above followed by number of bytes: 10 xxxxxx
        else:
            raise Exception("unicode too large")

    @staticmethod
    def from_bytes(u8):
        l = UTF8.byte_size(u8[0])
        value = 0x00
        if l == 1:
            value = value + u8[0]
        else:
            # head
            pat = 0xFF >> (l+1)
            value = value | (u8[0] & pat)  # head byte
            for i in range(1, l):
                value = value << 6
                value = value | (u8[i] & 0x3F)  # following byte
        return value

    @staticmethod
    def to_bytes(value):
        size = UTF8.unicode_byte_size(value)
        u8 = bytearray(size)
        for i in range(size-1, 0, -1):
            # last 6 bit
            u8[i] = 0x80 | (value & 0x3F)
            value = value >> 6
            pass
        if size == 1:
            u8[0] = value
            return u8 # ascii
        lead_bits = (0xFE << (7 - size)) & 0xFF # 0b11111110 << size
        u8[0] = lead_bits | value
        return u8

def is_special_ascii_code(code: int):
    if code in (ASCII_R, ASCII_N, ASCII_T, ASCII_SPACE):
        return True
    return False

def is_display_ascii_code(code: int):
    if code >= ASCII_DATA_START and code <= ASCII_DATA_END:
        return True
    return False

def is_ascii_code(code: int) -> bool:
    if is_special_ascii_code(code):
        return True
    if is_display_ascii_code(code):
        return True
    return False

def encode_text(text: str) -> Tuple[Dict[int, str], bytes]:
    char_dict = dict()
    # get all chars
    for ch in text:
        if is_ascii_code(ord(ch)):
            pass # reserved code
        else:
            if ch not in char_dict:
                char_dict[ch] = 0
            char_dict[ch] += 1
    # sort chars
    chars = list(char_dict.keys())
    chars.sort(key=lambda c: char_dict[c], reverse=True)
    # build new char encoding
    code = 1
    new_char_code = dict()
    for ch in chars:
        new_char_code[ch] = code
        code += 1
        while is_ascii_code(code):
            # deal with reserved code
            new_char_code[chr(code)] = code
            code += 1
    # encode with new char encoding
    mapped = bytearray()
    for ch in text:
        mapped.extend(UTF8.to_bytes(new_char_code[ch]))
    new_char_code_r = { v: k for k, v in new_char_code.items() }
    compressed = zlib.compress(mapped, level=9, wbits=9)
    # # preview text
    # print("================")
    # pos = 0
    # chars = list()
    # while pos < len(mapped):
    #     bsize = UTF8.byte_size(mapped[pos])
    #     code = UTF8.from_bytes(mapped[pos: pos+bsize])
    #     if is_special_ascii_code(code):
    #         chars.append(chr(code))
    #     else:
    #         chars.append(new_char_code_r[code])
    #     pos += bsize
    # print("".join(chars))
    # for b in mapped[:38]:
    #     print(hex(b))
    print("================")
    print("utf-8编码大小:", len(text.encode("utf-8")))
    print("gb18030编码大小:", len(text.encode("gb18030")))
    print("优化编码+压缩后大小:", len(compressed))
    print("压缩率: {:.2f}%".format((len(compressed) / len(text.encode("utf-8"))) * 100))
    print("全文字符个数:", len(text))
    print("不同字符个数:", len(new_char_code))
    return new_char_code_r, bytes(compressed)

def encode_text_substring(char_code_map: Dict[int, str], substring) -> bytes:
    new_char_code = { v: k for k, v in new_char_code.items() }
    mapped = bytearray()
    for ch in substring:
        mapped.extend(UTF8.to_bytes(new_char_code[ch]))
    return bytes(mapped)

def make_font_bin(char_code_map: Dict[int, str], font_file: str) -> bytes():
    """
    font_width: 1 byte
    font_height: 1 byte
    ascii_width: 94 bytes
    data_block_size: 1 byte
    char_count: 2 bytes
    reserve: 1 byte
    block_end_offset: 2 bytes * ((char_count / data_block_size) + 1)
    blocks_of_data: n bytes * char_count
    ========
    char_index = char_code - 1
    block_end_offset is each block end offset
    """
    DATA_BLOCK_CHAR_COUNT = 128
    data = bytearray()
    with open(font_file, "rb") as font_stream:
        if font_file == FONT8_FILE:
            ascii_width = FONT8_ASCII_WIDTH
        elif font_file == FONT16_FILE:
            ascii_width = FONT16_ASCII_WIDTH
        else:
            ascii_width = bytes()
        font = UnicodeBitmapFont(font_stream, ascii_width)
        data.append(font.get_font_size()[0]) # font width
        data.append(font.get_font_size()[1]) # font height
        data.extend(bytes([ font.get_char_width(c) for c in range(ASCII_DATA_START, ASCII_DATA_END + 1) ])) # ascii char width
        data.append(DATA_BLOCK_CHAR_COUNT) # data block size (n chars' data)
        data.extend(int.to_bytes(len(char_code_map), 2, "big")) # char count
        data.append(0) # reserve
        # generate font data
        blocks = dict()
        block_data = bytearray()
        block_id = 0
        total1 = 0
        total2 = 0
        for i in range(len(char_code_map)):
            ch = char_code_map[i+1]
            char_data = font.get_char_data(ord(ch))
            # char preview
            # if i < 64:
            #     print(f"======== {ch} {i+1} ========")
            #     if (font.get_font_size()[0] == 8):
            #         for d in char_data:
            #             print(f"{d:08b}".replace("0", " ").replace("1", "█"))
            #     elif (font.get_font_size()[0] == 16):
            #         for idx in range(0, 32, 2):
            #             print(f"{char_data[idx]:08b}".replace("0", " ").replace("1", "█"), end="")
            #             print(f"{char_data[idx+1]:08b}".replace("0", " ").replace("1", "█"))
            if i % DATA_BLOCK_CHAR_COUNT == 0:
                if i != 0:
                    # print("未压缩字体大小:", len(block_data))
                    total1 += len(block_data)
                    compressed = zlib.compress(block_data, level=9, wbits=9)
                    # print("压缩字体大小:", len(compressed))
                    total2 += len(compressed)
                    blocks[block_id] = (total2, compressed)
                    block_data.clear()
                    block_id += 1
            block_data.extend(char_data)
        # last block
        total1 += len(block_data)
        compressed = zlib.compress(block_data, level=9, wbits=9)
        total2 += len(compressed)
        blocks[block_id] = (total2, compressed)
        # make bin
        block_count = (len(char_code_map) // DATA_BLOCK_CHAR_COUNT) + 1
        for i in range(block_count):
            offset = blocks[i][0]
            assert offset < 0xFFFF
            data.extend(int.to_bytes(offset, 2, "big")) # block size
        for i in range(block_count):
            char_data = blocks[i][1]
            data.extend(char_data) # char data
        final_size = len(data)
        print("================")
        print("字库 压缩/未压缩:", final_size, "/", total1)
        print("压缩率: {:.2f}%".format((final_size / total1) * 100))
        return bytes(data)

def make_noval_bin(text_path) -> bytes:
    """
    data_len_n: 2 bytes
    data: n bytes
    ========
    """
    data = bytearray()
    with open(text_path, "rb") as f:
    # with open("魔法密林.txt", "rb") as f:
        text = f.read().decode("utf-8")
    new_char_code, encoded_text = encode_text(text)
    font_bin = make_font_bin(new_char_code, FONT16_FILE)
    data.extend(int.to_bytes(len(encoded_text), 2, "big")) # text data size
    data.extend(encoded_text) # text data
    data.extend(int.to_bytes(len(font_bin), 2, "big")) # font data size
    data.extend(font_bin) # font data
    print("================")
    print("总数据大小:", len(data))
    return bytes(data)

def bin_to_c_header(data: bytes, name: str) -> str:
    f = io.StringIO()
    f.write("#pragma once\n")
    f.write("#include <stdint.h>\n")
    f.write("\n")
    f.write(f"const uint8_t DATA_{name.upper()}[] = {{")
    for i, byte in enumerate(data):
        if i % 16 == 0:
            f.write("\n   ")
        f.write(f" 0x{byte:02X},")
    f.write("\n};\n")
    return f.getvalue()

def main():
    data = make_noval_bin("sometext.txt")
    header_content = bin_to_c_header(data, "NOVAL")
    with open("_compressed_data.h", "wb") as f:
        f.write(header_content.encode("utf-8"))

if __name__ == "__main__":
    main()