# kicad-bom

A set of tools for exporting manual assembly oriented BOMs from KiCAD:
 - `read_pcb.py <PCB>` - Lists all footprints on a PCB and sorts them. Columns are separated with tabs.
 - `to_csv.awk`, `to_gist.awk` - Formats output from `read_pcb.py`
 - `create_gist.py <PCB>` - Creates a private Gist with all generated files
