import os
import gzip

def after_install():
    lib_dir = os.path.dirname(__file__)
    src_file = os.path.join(lib_dir, "..", "src", "index.html")
    temp_gz_file = os.path.join(lib_dir, "..", "src", "index.html.gz")
    dest_header_file = os.path.join(lib_dir, "..", "include", "index_html_gz.h")

    if not os.path.exists(src_file):
        print(f"Error: {src_file} not found in library src directory")
        return

    try:
        with open(src_file, "rb") as f_in:
            with gzip.open(temp_gz_file, "wb", compresslevel=9) as f_out:
                f_out.writelines(f_in)
        print(f"Successfully compressed {src_file} to {temp_gz_file}")
    except Exception as e:
        print(f"Error compressing {src_file} to {temp_gz_file}: {str(e)}")
        return

    try:
        with open(temp_gz_file, "rb") as f_gz:
            gz_data = f_gz.read()

        hex_lines = []
        for i in range(0, len(gz_data), 16):
            line = ", ".join(f"0x{b:02X}" for b in gz_data[i:i+16])
            hex_lines.append(f"    {line}" + ("," if i + 16 < len(gz_data) else ""))
        hex_data = "\n".join(hex_lines)
        
    except Exception as e:
        print(f"Error reading {temp_gz_file}: {str(e)}")
        return

    header_content = f"""#ifndef INDEX_HTML_GZ_H
#define INDEX_HTML_GZ_H

#include <pgmspace.h>

// Binary data of index.html.gz
const unsigned char index_html_gz_data[] PROGMEM = {{
{hex_data}
}};

// Length of the gzip-compressed data
const size_t index_html_gz_len = sizeof(index_html_gz_data);

#endif
"""

    try:
        with open(dest_header_file, "w") as f_out:
            f_out.write(header_content)
        print(f"Successfully created {dest_header_file}")
    except Exception as e:
        print(f"Error writing {dest_header_file}: {str(e)}")
        return

    try:
        os.remove(temp_gz_file)
        print(f"Removed temporary file: {temp_gz_file}")
    except Exception as e:
        print(f"Error removing {temp_gz_file}: {str(e)}")

if __name__ == "__main__":
    after_install()
