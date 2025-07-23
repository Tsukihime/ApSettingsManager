import os
import gzip
import platformio.project.helpers as pio_helpers

def after_install():
    # Get paths
    project_dir = pio_helpers.get_project_dir()
    lib_dir = os.path.dirname(__file__)
    src_file = os.path.join(lib_dir, "..", "src", "index.html")
    dest_data_dir = os.path.join(project_dir, "data")
    dest_file = os.path.join(dest_data_dir, "index.html.gz")

    # Check if source file exists
    if not os.path.exists(src_file):
        print(f"Error: {src_file} not found in library src directory")
        return

    # Create data directory in project if it doesn't exist
    if not os.path.exists(dest_data_dir):
        os.makedirs(dest_data_dir)
        print(f"Created directory: {dest_data_dir}")

    # Compress index.html to index.html.gz
    try:
        with open(src_file, "rb") as f_in:
            with gzip.open(dest_file, "wb", compresslevel=9) as f_out:
                f_out.writelines(f_in)
        print(f"Successfully compressed {src_file} to {dest_file}")
    except Exception as e:
        print(f"Error compressing {src_file} to {dest_file}: {str(e)}")

if __name__ == "__main__":
    after_install()
    