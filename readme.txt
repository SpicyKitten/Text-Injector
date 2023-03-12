1. install clang format tool:
sudo apt install clang-format

2. use clang-format to format the C file:
clang-format prog_raw.c > prog_formatted.c

3. python parse_func.py
This gives an example of finding the injectable locations. You can read the code and finish the rest of the part.

Here is an example of resultant interface, which needs to inject a snippet to a source file at one/many locations.

augment([src path], [snippet path])

input:
src.c
snippet.c/snippet.txt

output:
generate some c files after augmentation with proper naming

src_aug_1.c
src_aug_2.c
...


Overall augmentation input and output:

input:
./sources/{src_1/{nonvul.c}, src_2/{nonvul.c}, ...}
./snippets/{snippet1/{snippet1_nonvul.c, snippet1_vul.c}, snippet2/{snippet2_nonvul.c, snippet2_vul.c}}

output:
source_list.txt  containing M sample names
snippet_list.txt containing N snippet names 
./augmented/{src_m_snippet_n_k.c, ...} 
where src_m_snippet_n_k.c means augment src_m with snippet_n, k denotes variation of injected location
