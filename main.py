import os
import random
import shutil

config = {
    'SOURCE_FOLDER': 'sources',
    'SNIPPET_FOLDER': 'snippets',
    'OUTPUT_FOLDER': 'augmented',
    'SOURCE_PREFIX': 'src_',
    'SNIPPET_PREFIX': 's',
    'INDENT_SIZE': 4,
    'TAB_SIZE': 8,
    'MAX_INJECTIONS_PERFORMED': 3
}


def num_tab(line):
    assert isinstance(line, str)
    tab_count = 0
    for character in line:
        if character == '\t':
            tab_count += 1
        else:
            break
    return tab_count


def indent_size(line):
    assert isinstance(line, str)
    size = 0
    for character in line:
        if character == ' ':
            size += 1
        elif character == '\t':
            size += config['TAB_SIZE']
        else:
            break
    return size


def is_empty_line(line):
    assert isinstance(line, str)
    for character in line:
        if character not in [' ', '\t', '\n']:
            return False
    return True


def compact(line):
    assert not is_empty_line(line)
    start = None
    end = None
    N = len(line)
    for i in range(N):
        if line[i] not in [' ', '\t', '\n']:
            start = i
            break
    for i in range(N):
        if line[N - 1 - i] not in [' ', '\t', '\n']:
            end = N - i
            break

    if start != None and end != None:
        return line[start:end]
    else:
        return ''


def get_segments(source):
    idt_sizes = [indent_size(line) for line in source]

    segments = [[]]

    for i, line in enumerate(source):
        if i == 2 and source[1] == '{\n':
            segments.append([])
            segments[-1].append(line)
            continue
        if i > 2 and idt_sizes[i] == config['INDENT_SIZE'] and not is_empty_line(line):
            j = i - 1
            flag = False
            while (j >= 0):
                if not is_empty_line(source[j]):
                    flag = True
                    break
                else:
                    j = j - 1
            if flag:
                prev = source[j].strip('\n')
                if idt_sizes[j] == config['INDENT_SIZE']:
                    if compact(prev).endswith(';') or compact(prev).endswith('}'):
                        segments.append([])
        segments[-1].append(line)
    segments = [''.join(segment) for segment in filter(None, segments)]
    return [segment for segment in segments if segment and not segment.isspace()]


def inject(segments, snippet, injection_location, output_files):
    (nonvul_snippet, vul_snippet) = snippet
    (output_file, vulnerable_output_file) = output_files
    segment_index = -1
    indent = ' '*config['INDENT_SIZE'];
    print(f"/* Injection at location {injection_location} */", file=output_file)
    print(f"/* Injection at location {injection_location} */", file=vulnerable_output_file)
    for segment in segments:
        if segment_index == injection_location:
            print("{indent}/* Inserted block */", file=output_file)
            print("{indent}/* Inserted block */", file=vulnerable_output_file)
            for line in nonvul_snippet:
                print(indent + line, end='', file=output_file)
            for line in vul_snippet:
                print(indent + line, end='', file=vulnerable_output_file)
            print("{indent}/* End of injection */", file=output_file)
            print("{indent}/* End of injection */", file=vulnerable_output_file)
        for line in segment:
            print(line, file=output_file, end='')
        for line in segment:
            print(line, file=vulnerable_output_file, end='')
        segment_index += 1
    pass


def augment(source, source_name, snippet, snippet_name):
    outputs_path = os.path.join(os.path.abspath(
        os.path.dirname(__file__)), config['OUTPUT_FOLDER'])
    segments = get_segments(source)
    injectable_locations = len(segments) - 1
    print(f"Injectable locations: {injectable_locations}")
    injection_locations = random.sample(range(injectable_locations), min(
        config['MAX_INJECTIONS_PERFORMED'], injectable_locations))
    print(f"Injecting at locations: {injection_locations}")
    injection = -1
    for injection_location in injection_locations:
        injection += 1
        output_file_path = os.path.join(
            outputs_path, f'{config["SOURCE_PREFIX"]}{source_name}_{config["SNIPPET_PREFIX"]}{snippet_name}_{injection}_nonvul.c')
        vulnerable_output_file_path = os.path.join(
            outputs_path, f'{config["SOURCE_PREFIX"]}{source_name}_{config["SNIPPET_PREFIX"]}{snippet_name}_{injection}_vul.c')
        with open(output_file_path, 'w+') as output_file, open(vulnerable_output_file_path, 'w+') as vulnerable_output_file:
            inject(segments, snippet, injection_location, (output_file, vulnerable_output_file))


def main():
    # THIS WORKS
    sources_path = os.path.join(os.path.abspath(
        os.path.dirname(__file__)), config['SOURCE_FOLDER'])
    snippets_path = os.path.join(os.path.abspath(
        os.path.dirname(__file__)), config['SNIPPET_FOLDER'])
    outputs_path = os.path.join(os.path.abspath(
        os.path.dirname(__file__)), config['OUTPUT_FOLDER'])
    source_prefix = config['SOURCE_PREFIX']
    snippet_prefix = config['SNIPPET_PREFIX']

    for root, dirs, files in os.walk(outputs_path):
        for f in files:
            os.unlink(os.path.join(root, f))
        for d in dirs:
            shutil.rmtree(os.path.join(root, d))
    for path in os.scandir(sources_path):
        if path.name.startswith(source_prefix) and len(path.name) > len(source_prefix):
            source_name = path.name[len(source_prefix):]
            source_path = os.path.join(sources_path, path.name, 'nonvul.c')
            dest_path = os.path.join(sources_path, path.name, 'formatted.c')
            with open(source_path, 'r') as _:
                os.system(
                    f"clang-format {source_path} -style=file:./.clang-format > {dest_path}")
            with open(dest_path, 'r') as source_file:
                source = source_file.readlines()
                for path in os.scandir(snippets_path):
                    if path.name.startswith(snippet_prefix) and len(path.name) > len(snippet_prefix):
                        snippet_name = path.name[len(snippet_prefix):]
                        nonvul_snippet_path = os.path.join(
                            snippets_path, path.name, f'{path.name}_nonvul.c')
                        vul_snippet_path = os.path.join(
                            snippets_path, path.name, f'{path.name}_vul.c')
                        with open(nonvul_snippet_path, 'r') as nonvul_snippet_file, \
                                open(vul_snippet_path, 'r') as vul_snippet_file:
                            nonvul_snippet = nonvul_snippet_file.readlines()
                            vul_snippet = vul_snippet_file.readlines()
                            augment(source, source_name,
                                    (nonvul_snippet, vul_snippet), snippet_name)
    print(sources_path)
    print(snippets_path)

    # os.system("clang-format ./inputs/prog.c > ./inputs/prog_formatted.c")
    pass


if __name__ == '__main__':
    main()
