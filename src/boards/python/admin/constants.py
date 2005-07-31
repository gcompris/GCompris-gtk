#  gcompris - constants.py
# 
# Copyright (C) 2005 Bruno Coudoin and Yves Combe
# 
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
# 
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
# 
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
# 


# Some common constants for the administration

COLUMN_WIDTH_LOGIN                   = 80
COLUMN_WIDTH_FIRSTNAME               = 100
COLUMN_WIDTH_LASTNAME                = 180
COLUMN_WIDTH_BIRTHDATE               = 40

COLUMN_WIDTH_CLASSNAME               = 100
COLUMN_WIDTH_TEACHER                 = 300

COLUMN_WIDTH_GROUPNAME               = 100
COLUMN_WIDTH_GROUPDESCRIPTION        = 200
COLUMN_WIDTH_GROUPDESCRIPTION_EDIT   = 150

COLUMN_WIDTH_PROFILENAME             = 100
COLUMN_WIDTH_PROFILEDESCRIPTION      = 300

#
# Some utility method
# -------------------

# Return the next class id in the base
# Params are db_connect, db_cursor
def get_next_class_id(con, cur):
    cur.execute('select max(class_id) from class')
    class_id = cur.fetchone()[0]
    
    if(class_id == None):
        class_id=0
    else:
        class_id += 1
      
    return class_id



# Return the next group id in the base
# Params are db_connect, db_cursor
def get_next_group_id(con, cur):
    cur.execute('select max(group_id) from groups')
    group_id = cur.fetchone()[0]
    
    if(group_id == None):
        group_id=0
    else:
        group_id += 1
      
    return group_id


# Return the next profile id in the base
# Params are db_connect, db_cursor
def get_next_profile_id(con, cur):
    cur.execute('select max(profile_id) from profiles')
    profile_id = cur.fetchone()[0]
    
    if(profile_id == None):
        profile_id=0
    else:
        profile_id += 1
        
    return profile_id


# get_wholegroup_id
# From the given class_id, return it's wholegroup_id
# Params are db_connect, db_cursor, class_id
def get_wholegroup_id(con, cur, class_id):
    cur.execute('SELECT wholegroup_id FROM class WHERE class_id=?',
                (class_id,))
    return(cur.fetchone()[0])


# get_class_name_for_group_id
# From the given group_id, return it's class name
# Or "" if not found
def get_class_name_for_group_id(con, cur, group_id):

    class_name = ""
    
    # Extract the class name of this group
    cur.execute('SELECT class_id FROM groups WHERE group_id=?',
                     (group_id,))
    result = cur.fetchall()
    if(result):
        class_id = result[0][0]

        cur.execute('SELECT name FROM class WHERE class_id=?',
                    (class_id,))
        result = cur.fetchall()
        if(result):
            class_name = result[0][0]

    return class_name
