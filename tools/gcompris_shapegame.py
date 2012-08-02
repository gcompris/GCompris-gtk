#!/usr/bin/python
#
# Copyright (C) 2006 Miguel DE IZARRA
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, see <http://www.gnu.org/licenses/>.
#
#######################################################################
#
# a Python-Fu Gimp script for GCompris Shapegame activities
#
# Require: gimp-python
#
# To install, copy this script in ~/.gimp-2.2/plug-ins/ (and make it is executable)
# cp gcompris_shapegame.py ~/.gimp-2.2/plug-ins/
# chmod +x ~/.gimp-2.2/plug-ins/gcompris_shapegame.py
#
#######################################################################
#
# To create a Puzzle:
# 1. Open an image in Gimp
# 2. Save the image
# 3. Menu Python-Fu -> GCompris -> Puzzle
# 4. Choice number of tile horizontaly, verticaly and type of puzzle
# 5. The resulting data are saved under the location of the original image.
# 6. Move the data under /usr/share/gcompris/boards/<paintings>
#
#######################################################################
#
# To create a Geography map:
# 1. Open a map in Gimp
# 2. Save the image in Gimp Format (.xcf) to preserve channels
# 3. For each region:
#   a. Select individual region
#   b. Save selection to channel
#   c. Rename channel in the name of region
#  (You can create a channel for Foreign regions)
# 4. Menu Python-Fu -> GCompris -> Geography
#   a. Choice output directory and title (default based on image name)
#   b. Set the border size between regions (depend on the original map)
#   c. Choice if borders between region should be show
# 5. The resulting data are saved under the location of the original image.
# 6. Move the data under /usr/share/gcompris/boards/<geography>
#
# We can change color of a region by opening region file in gimp,
# and fill it by a new color.
#

from gimpfu import *
import os.path
import math
from random import randint

def gcompris_init(img):
    if len(img.layers) > 1:
        gimp.message("Error : There too many layers")
        return False

    if not img.filename:
        gimp.message("Error : save the image first (in Gimp .xcf format)")
        return False

    # resize the image
    ratio = max(img.height / 520.0, img.width / 800.0)
    if ratio > 1:
        new_w, new_h = [ int(i / ratio) for i in (img.width, img.height) ]
        pdb.gimp_image_scale(img, new_w, new_h)
    return True

def gcompris_puzzle(img, sdrawable, x, y, activity_name,
                    folder, title, puzzle_type,
                    level, sublevel):
    """ Create file png and board.xml from current image """

    def puzzle_layer(img, ptype, x, y):
        """ Create a puzzle layer
            Create x tile horizontaly, y tile verticaly
            ptype = 0 -> normal, 1 -> rectanglular (grid)
        """
        width, height = img.width, img.height
        layer = gimp.Layer(img, "puzzle", width, height, RGB_IMAGE,
            100, NORMAL_MODE)
        layer.fill(WHITE_FILL)
        img.add_layer(layer)
        if ptype:
            vspace = math.ceil(width / float(x))
            hspace = math.ceil(height / float(y))
            pdb.plug_in_grid(img, layer, 1, hspace, 0, (0, 0, 0), 255,
                1, vspace, 0, (0, 0, 0), 255,
                0, 0, 0, (0, 0, 0), 255)
        else:
            pdb.plug_in_jigsaw(img, layer, x, y, 1, 0, 0)
        return layer

    if not gcompris_init(img):
        return
    pdb.gimp_image_scale(img, img.width*2, img.height*2)
    # Default values
    if(not activity_name):
        activity_name = "paintings"

    # make float to int
    x = int(x)
    y = int(y)

    # Init
    bg_layer = img.active_layer
    pdb.gimp_selection_none(img)
    width, height = img.width, img.height
    # Create the puzzle layer
    layerlist = []
    puzzle = puzzle_layer(img, puzzle_type, x, y)
    for idy in xrange(y):
        posy = height / y * (.5 + idy)
        for idx in xrange(x):
            posx = width / x * (.5 + idx)
            pdb.gimp_fuzzy_select(puzzle, posx, posy, 0, CHANNEL_OP_REPLACE,
                True, 0, 0, 0)
            pdb.gimp_selection_grow(img, 2)
            pdb.gimp_selection_feather(img, 3)
            pdb.gimp_edit_copy(bg_layer)
            pdb.gimp_floating_sel_to_layer(pdb.gimp_edit_paste(bg_layer, 1))
            pdb.script_fu_add_bevel(img,img.active_layer,10,0,0)
            img.active_layer.name = chr(ord('A')+ idx) + str(idy+1)
            layerlist.append(img.active_layer)

    pdb.gimp_selection_none(img)
    img.remove_layer(puzzle)

    d_title = dict(x=405, y=495, justification ="GTK_JUSTIFY_CENTER",
        color_skin="gcompris/text button")
    d_title["<name>"] = title
    shapebg=[]
    # shapebg = dict(name=1, pixmapfile="skin:gcompris-shapelabel.png",
    #    type="SHAPE_BACKGROUND", x=405, y=495, position=0)
    pdb.gimp_image_scale(img, img.width/2, img.height/2)
    gcompris_layer_to_board(layerlist, activity_name, folder, d_title, shapebg, None,
                            level, sublevel)

register(
    "gcompris_puzzle",
    "Make Puzzle for GCompris ",
    "Make Puzzle for GCompris ",
    "Miguel de Izarra",
    "Miguel de Izarra",
    "2006",
    "<Image>/Python-Fu/GCompris/Puzzle",
    "RGB*, GRAY*",
    [
        (PF_SPINNER, "x", "Number of tiles across", 3, (2, 9, 1) ),
        (PF_SPINNER, "y", "Number of tiles down", 3, (2, 9, 1) ),
        (PF_STRING, "activity_name", "Name of the activity (default = painting)", ""),
        (PF_STRING, "folder", "Relative output directory (default = image name)", ""),
        (PF_STRING, "title", "GCompris puzzle title", ""),
        (PF_SPINNER, "type", "Type of puzzle (0 Normal 1 Rectangle)", 0, (0, 1, 1) ),
        (PF_SPINNER, "level", "The level in the activity", 1, (1, 9, 1) ),
        (PF_SPINNER, "sublevel", "The sub level in the activity", 0, (0, 20, 1) )
    ],
    [],
    gcompris_puzzle)

def gcompris_geography(img, sdrawable, activity_name,
                       folder, title, bordersize, keepLimit, level, sublevel):
    """Create file for the geography activity from the current image """
    # Default values
    if(not activity_name):
        activity_name = "geography"

    if not gcompris_init(img):
        return
    if len(img.channels) == 0:
        gimp.message("Can't find channels. Please create them.")
        return

    save_bg = gimp.get_background()
    save_fg = gimp.get_foreground()

    layer_map = img.layers[0]
    if not layer_map.has_alpha:
    	layer_map.add_alpha()

    layerlist = []
    pdb.gimp_selection_none(img)
    all_channel = pdb.gimp_selection_save(img)
    all_channel.name = "all_channel"
    for chan in img.channels :
        if chan == all_channel:
            continue
        pdb.gimp_selection_load(chan)
        if not keepLimit:
            pdb.gimp_selection_grow(img, bordersize)
        # Save selection in all_channel
        pdb.gimp_edit_fill(all_channel, WHITE_FILL)
        # create new layer from selection
        pdb.gimp_edit_copy(layer_map)
        pdb.gimp_floating_sel_to_layer(pdb.gimp_edit_paste(layer_map, 1))
        layer = img.active_layer
        # Fill a random color
        gimp.set_background(randint(0, 255), randint(0, 255), randint(0, 255))
        pdb.gimp_edit_fill(layer, BACKGROUND_FILL)
        layer.name = chan.name
        layerlist.append(layer)

    pdb.gimp_context_set_default_colors()
    # Fill country white
    pdb.gimp_selection_load(all_channel)
    pdb.gimp_edit_fill(layer_map, BACKGROUND_FILL)

    pdb.gimp_selection_grow(img, bordersize)
    earth = pdb.gimp_selection_save(img)
    earth.name = "earth"

    # Fill limit black
    pdb.gimp_selection_combine(all_channel, CHANNEL_OP_SUBTRACT)
    pdb.gimp_edit_fill(layer_map, FOREGROUND_FILL)  # Fill Black

    # Clear other
    pdb.gimp_selection_load(earth)
    pdb.gimp_selection_invert(img)
    pdb.gimp_edit_clear(layer_map)

    # Water 1
    pdb.gimp_selection_load(earth)
    pdb.gimp_selection_grow(img, 10)
    pdb.gimp_selection_combine(earth, CHANNEL_OP_SUBTRACT)
    gimp.set_foreground(58, 68, 219)
    gimp.set_background(94, 146, 229)
    pdb.gimp_edit_blend(layer_map, 0, 0, 0, 50, 0, 0, False, False, 0, 0,
                        True, 0, 0, img.width, img.height)

    # Water 2
    pdb.gimp_selection_load(earth)
    pdb.gimp_selection_grow(img, 20)
    pdb.gimp_selection_combine(earth, CHANNEL_OP_SUBTRACT)
    gimp.set_foreground(110, 123, 215)
    gimp.set_background(137, 173, 225)
    pdb.gimp_edit_blend(layer_map, 0, 0, 0, 25, 0, 0, False, False, 0, 0,
                        True, 0, 0, img.width, img.height)

    # Remove tmp channel
    img.remove_channel(earth)
    img.remove_channel(all_channel)

    gimp.set_background(save_bg)
    gimp.set_foreground(save_fg)
    pdb.gimp_selection_none(img)

    shape = dict()
    shape["sound"] = os.path.join("sounds", "$LOCALE", "geography", folder, "%n.ogg")
    shape["<tooltip>"] = "%n"
    title_d = dict(x=600, y=495, justification="GTK_JUSTIFY_CENTER")
    title_d["<name>"] = title

    layer_map.name = "background"
    gcompris_layer_to_board(layerlist, activity_name, folder, title_d, layer_map, shape,
                            level, sublevel)

register(
    "gcompris_geography",
    "Make files for geography activity",
    "Make files for geography activity",
    "Miguel de Izarra",
    "Miguel de Izarra",
    "2006",
    "<Image>/Python-Fu/GCompris/Geography",
    "RGB*, GRAY*",
    [
        (PF_STRING, "activity_name", "Name of the activity (default = geography)", ""),
        (PF_STRING, "folder", "Relative output directory (default = image name)", ""),
        (PF_STRING, "title", "GCompris puzzle title", ""),
        (PF_SPINNER, "bordersize", "The Size of border between country", 1, (1, 20, 1) ),
        (PF_TOGGLE, "keepLimit", "Keep limit between country", False),
        (PF_SPINNER, "level", "The level in the activity", 1, (1, 100, 1) ),
        (PF_SPINNER, "sublevel", "The sub level in the activity", 0, (0, 20, 1) ),
    ],
    [],
    gcompris_geography)

def gcompris_babyshapes(img, sdrawable, activity_name,
                       folder, title, bordersize, shapes_color,level, sublevel):
    """Create file for the babyshapes activity from the current image """
    # Default values
    if(not activity_name):
        activity_name = "babyshapes"

    if not gcompris_init(img):
        return
    if len(img.channels) == 0:
        gimp.message("Can't find channels. Please create them.")
        return

    save_bg = gimp.get_background()
    save_fg = gimp.get_foreground()

    layer_map = img.layers[0]
    if not layer_map.has_alpha:
    	layer_map.add_alpha()

    layerlist = []
    for chan in img.channels :
        pdb.gimp_selection_load(chan)
        pdb.gimp_selection_grow(img, bordersize)
        # create new layer from selection
        pdb.gimp_edit_copy(layer_map)
        pdb.gimp_floating_sel_to_layer(pdb.gimp_edit_paste(layer_map, 1))
        layer = img.active_layer
        layer.name = chan.name
        layerlist.append(layer)
		# Fill main image with a color
        if shapes_color==1:
	        gimp.set_background(255,255, 255)
        elif shapes_color==2:
   	        gimp.set_background(0, 0, 0)
        else:
            gimp.set_background(randint(0, 255), randint(0, 255), randint(0, 255))
        pdb.gimp_selection_load(chan)
        pdb.gimp_selection_grow(img, bordersize)
        pdb.gimp_edit_fill(layer_map,BACKGROUND_FILL)		


    gimp.set_background(save_bg)
    gimp.set_foreground(save_fg)
    pdb.gimp_selection_none(img)

    shape = dict()
    shape["<tooltip>"] = "%n"
    title_d = dict(x=600, y=495, justification="GTK_JUSTIFY_CENTER")
    title_d["<name>"] = title

    layer_map.name = "background"
    gcompris_layer_to_board(layerlist, activity_name, folder, title_d, layer_map, shape,
                            level, sublevel)

register(
    "gcompris_babyshapes",
    "Make files for babyshapes activity",
    "Make files for babyshapes activity",
    "Miguel de Izarra. Adapted by Angel Ivan Moreno",
    "Miguel de Izarra. Adapted by Angel Ivan Moreno",
    "2012",
    "<Image>/Python-Fu/GCompris/Babyshapes",
    "RGB*, GRAY*",
    [
        (PF_STRING, "activity_name", "Name of the activity (default = babyshapes)", ""),
        (PF_STRING, "folder", "Relative output directory (default = image name)", ""),
        (PF_STRING, "title", "GCompris puzzle title", ""),
        (PF_SPINNER, "bordersize", "The Size of border ", 0, (0, 20, 1) ),
		(PF_OPTION,"shapes_color",   "Background color of shapes:", 0, ["Random","White","Black"]),
        (PF_SPINNER, "level", "The level in the activity", 1, (1, 100, 1) ),
        (PF_SPINNER, "sublevel", "The sub level in the activity", 0, (0, 20, 1) ),
    ],
    [],
    gcompris_babyshapes)
	
	
def gcompris_layer_to_board(layerlist, activity, subdir, title, background, shape,
                            level, sublevel):
    """ Create png file and board.xml for gcompris
    layerlist : the list of layer to process
    activity : gcompris activity name in which boardx_y.xml will be put
    subdir : sub directory in which png files will be put
    title : a dictionnary with name and value of properties of element <title>
    background : a dictionnary with name and value of properties of the Shape Background
        or a background Layer
    shape : a dictonnary of extra properties of each shape. In values, "%n" replace the layer name / filename
    level : The level in the activity
    sublevel : The sub level in the activity
    """
    def realbasename(f):
        """Return the basename without extension"""
        return os.path.splitext(os.path.basename(f))[0]

    def dict_to_str(name, dico):
        """ Convert a dictionnary to string of a xml element
        dictionnary'keys are convert to a property of the xml element with associate value
        dictionnary'keys starting by < and ending by > are add as subeleemnt
        """
        tmp = "<"+name+" "
        attrlist = [ k+'="'+str(v)+'"' for k, v in dico.iteritems() if not k.startswith("<") ]
        attrlist.sort()
        tmp += " ".join(attrlist)
        elementlist = [ (k[1:-1], v) for k, v in dico.iteritems() if k.startswith("<") ]
        if elementlist:
            tmp += ">\n"
            el_list = [ "<%s>%s</%s>" % (k, v, k) for k, v in elementlist]
            tmp += "\n".join(el_list)
            tmp += "</%s>\n" % name
        else:
            tmp += "/>\n"
        return tmp

    # Init
    img = layerlist[0].image
    width, height = img.width, img.height

    filename = img.filename
    if not subdir:
        subdir = realbasename(img.filename)
    out_dir = os.path.join(os.path.dirname(filename), activity, subdir)
    if not os.path.isdir(out_dir):
        if os.path.exists(out_dir):
            os.remove(out_dir)
        os.makedirs(out_dir)

    out_xml = os.path.join(out_dir, "..", "board%d_%d.xml" % (level, sublevel))
    xml = file(out_xml, "w")
    xml.write("""<?xml version="1.0" encoding="UTF-8"?>
<ShapeGame><!-- Created with GComprisShapegame gimp script -->\n""")

    if title:
        xml.write(dict_to_str("title", title))

    if isinstance(background, dict):
        xml.write(dict_to_str("Shape", background))
    elif isinstance(background, gimp.Layer):
        layerlist.append(background)

    deltax, deltay = (800-width)/2, (520-height)/2

    for tile_layer in layerlist:
        tile_filename = tile_layer.name.lower().replace(' ', '_')+".png"
        tile_filename_long = os.path.join(out_dir, tile_filename)
        pdb.file_png_save(img, tile_layer, tile_filename_long, tile_filename,
                          0, 0, 0, 0, 0, 0, 0)
        if shape:
            shape_dict = dict(shape)
            for key, value in shape_dict.iteritems():
                if "%n" in value:
                    shape_dict[key] = value.replace("%n", tile_layer.name)
        else:
            shape_dict = dict()
        offsx, offsy = tile_layer.offsets
        shape_dict["x"] = deltax + offsx + tile_layer.width / 2.0
        shape_dict["y"] = deltay + offsy + tile_layer.height /2.0
        shape_dict["position"] = 0
        shape_dict["pixmapfile"] = os.path.join(activity, subdir, tile_filename)
        shape_dict["name"] = tile_layer.name
        if tile_layer == background:
            shape_dict["type"] ="SHAPE_BACKGROUND"
        xml.write(dict_to_str("shape", shape_dict))
    xml.write("</ShapeGame>\n")
    xml.close()

main()

