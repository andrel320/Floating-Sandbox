import json
from operator import itemgetter
import sys

def main():
    
    if len(sys.argv) != 2:
        print("Usage: generate_materials_template.py <path_to_materials_json>")
        sys.exit(-1)

    with open(sys.argv[1]) as f:
        data = json.load(f)

    #
    # Prepare data
    #

    max_order = -1
    # order_dict: key=row_name,val=row_order    
    order_dict={}
    # d: key=row_name, val=list((col_order,col_name,Material))
    d={}
    
    for elem in data:
        
        row = elem["template"]["row"]
        row_parts = row.split("|")
        row_name = None
        row_order = None
        if len(row_parts) == 2:
            row_name = row_parts[1]
            row_order = int(row_parts[0])
        else:
            assert(len(row_parts) == 1)
            row_name = row_parts[0]
            
        old_order=order_dict.get(row_name)
        if old_order is not None and row_order != old_order:
            raise Exception("Conflicting order for row '" + row + "'; old order was '" + str(old_order) + "'")
        order_dict[row_name] = row_order
        max_order = max(max_order, row_order)

        col = str(elem["template"]["column"])
        col_parts = col.split("|")
        col_name = None
        col_order = None
        if len(col_parts) == 2:
            col_name = col_parts[1]
            col_order = int(col_parts[0])
        else:
            assert(len(col_parts) == 1)
            col_name = ""
            col_order = int(col_parts[0])

        d_vals = d.setdefault(row_name, [])
        d_vals.append((col_order,col_name,elem))

    # Fill-in None orders 
    for k,v in order_dict.iteritems():
        if v is None:
            max_order += 1
            order_dict[k] = max_order

    # Sort cols
    for k,v in d.iteritems():
        d[k] = sorted(v, key=itemgetter(0))


    #
    # Generate HTML
    #

    html = "<html><head><title>Materials Template</title>"
    html += "<style>tr.border_top td { border-top:1pt solid black;}</style>"
    html += "</head><body>"

    html += "<table style='border: 1px solid black' cellpadding=0 cellspacing=0>"

    def render_cols(col_values, func_render, func_empty):
        html = ""
        col_index = 0
        ci = 0
        while ci < len(col_values):
            col_value = col_values[ci]
            if col_value[0] == col_index:
                html += func_render(col_value)
                ci += 1
            else:
                html += func_empty()
            col_index += 1
        return html

    for k in sorted([x for x in d.iterkeys()], key=order_dict.get):

        # Colors
        html += "<tr>"
        html += "<td rowspan='3' valign='middle' style='padding-right:5px;font-size:10px;'>" + k + "</td>"
        
        html += render_cols(d[k], lambda col_value: (
            "<td bgcolor='" + col_value[2]["structural_colour"] + "'class='border_top' style='width: 50px;'>&nbsp;</td>"
            ), lambda: (
            "<td style='width: 50px;'>&nbsp;</td>"
            ))

        # Name
        html += "<tr>"
        html += render_cols(d[k], lambda col_value: (
            "<td style='font-size:8px;'>" + col_value[1] + "</td>"
            ), lambda: (
            "<td/>"
            ))
        html += "</tr>"

        # Data
        html += "<tr>"
        html += render_cols(d[k], lambda col_value: (
            "<td style='font-size:8px;'>" + str(col_value[2]["mass"]["nominal_mass"] * col_value[2]["mass"]["density"]) + "|" + str(col_value[2]["strength"]) + "|" + str(col_value[2]["stiffness"]) + "</td>"
            ), lambda: (
            "<td/>"
            ))
        html += "</tr>"

        
    html += "</table>";
    html += "</body></html>";

    with open("materials_template.html", "w") as html_file:
        html_file.write(html)

main()
