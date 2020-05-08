import argparse, string


def to_c_name(value):
    valid_chars = "-_%s%s" % (string.ascii_letters, string.digits)
    return "".join(c if c in valid_chars else '_' for c in value).upper()


def main():
    parser = argparse.ArgumentParser(description='Dump the bytes of some files into a cross platform c-compatible header file.')
    parser.add_argument('files', metavar='FILE', type=argparse.FileType('rb',0), nargs='+',
                        help='Input files to convert to variables')
    parser.add_argument('-o', '--output', metavar='data.h', type=argparse.FileType('w', encoding='UTF-8'),
                        default='data.h', help='The output header file')
                        
    args = parser.parse_args()
    
    for f in args.files:
        args.output.write("const unsigned char DATA_%s[] = {\n" %to_c_name(f.name))
        bytes = f.read()
        args.output.write( ", ".join( "0x%02x" % b for b in bytes ) + "\n")
        args.output.write("};\n")
    
    
    
if __name__ == '__main__':
    main()