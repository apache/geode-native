
def read_lines(filename):
    content = ()
    with open(filename) as f:
        content = f.readlines()
    # you may also want to remove whitespace characters like `\n` at the end of each line
    content = [x.rstrip() for x in content]

    return content


def scan_file(filename, callback):
    with open(filename) as f:
        for line in f:
            callback(line)


def scan_file_with_context(filename, callback, context):
    with open(filename) as f:
        for line in f:
            callback(line, context)
