
 // Copyright (C) 2005 Yves Combe
 //
 //   This program is free software; you can redistribute it and/or modify
 //   it under the terms of the GNU General Public License as published by
 //   the Free Software Foundation; either version 2 of the License, or
 //   (at your option) any later version.
 //
 //   This program is distributed in the hope that it will be useful,
 //   but WITHOUT ANY WARRANTY; without even the implied warranty of
 //   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 //   GNU General Public License for more details.
 //
 //   You should have received a copy of the GNU General Public License
 //   along with this program; if not, write to the Free Software
 //   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 //
 // Utility file for anim.py to export gcompris anim in Mozilla SVG

   var root;
   var frames_total;
   var frames_played;
   var AnimItemlist;
   var anim_running;
   var step = 0;

   // <script/> and <defs/>
   var base_stack=3;
   var frame_delay = 250;
   var fps = 4;


   var time_base;
   var next;

function init(){
  root = document.getElementById("svgroot");
  rate_text = document.getElementById("rate_text");
  speed_text = document.getElementById("speed_text");

  var elements;
  AnimItemlist = new Array (); // Array of array !

  var AnimItem ;
  var frames_info;
  var j;
  var k;
  var gc_frame, frame_attributs, gc_attr;
  var i = 0;

  while ( base_stack < root.childNodes.length ){
    // this loop keeps node <script> and the <defs/> elements.
    if  ((root.childNodes[i].nodeName != "rect") && (root.childNodes[i].nodeName != "ellipse") && (root.childNodes[i].nodeName != "use") && (root.childNodes[i].nodeName != "line" && (root.childNodes[i].nodeName != "text") )) {
       if (root.childNodes[i].nodeName == "gcompris:frames_total")
          frames_total = root.childNodes[i].getAttribute('value');
       if ((root.childNodes[i].nodeName != "script") && (root.childNodes[i].nodeName != "foreignObject") && (root.childNodes[i].nodeName != "defs")) {
          root.removeChild(root.childNodes[i]);
          }
          else i++;
       }
    else {
       AnimItem = new Array();

       element = root.childNodes[i];

       AnimItem.push(element);
       root.removeChild(element);

       frames_info = new Array();
       AnimItem.push(frames_info);

       for (j = 0; j < element.childNodes.length; j++){
          if ( element.childNodes[j].nodeType != 1) {
                 element.removeChild(element.childNodes[j]);
                 j--;
                 continue ;
                 }
          frame_attributs = new Array();
          gc_frame =  element.childNodes[j];
          for ( k=0;  k < gc_frame.attributes.length; k++){
	     gc_attr = new Array();
             gc_attr.push(gc_frame.attributes[k].nodeName);
	     gc_attr.push(gc_frame.attributes[k].nodeValue);
             frame_attributs.push(gc_attr);
             frame_attributs[gc_frame.attributes[k].nodeName]= frame_attributs[frame_attributs.length -1];
	     }
          if (gc_frame.hasChildNodes){
	    // text part of text element
            // supposed there is only one node, type 3 (text)
            frame_attributs['text'] = gc_frame.childNodes[0];
            }
          frames_info.push(frame_attributs);

          frames_info['frame_' + gc_frame.getAttribute('time')] = frames_info[frames_info.length -1];

          }

    AnimItem.push(frames_info);
    AnimItemlist.push(AnimItem);
  }
  }


  //for (k = 0; k < frames_total; k++){
  //  apply_frame(k);
  //  }
  //start_animation();
}

function apply_frame(frame_no){
   var i, j, k, gc_frame;

   for (i=0; i < AnimItemlist.length; i++) {
     // alert(AnimItemlist[i][0].nodeName + ' : ' + AnimItemlist.length );
     if (AnimItemlist[i][1]['frame_' + frame_no]){
         if (AnimItemlist[i][1]['frame_' + frame_no]['delete']){
	    root.removeChild(AnimItemlist[i][0]);
            continue;
            }
         if ( AnimItemlist[i][1]['frame_' + frame_no]['text'] ){
            //alert('text');
            // first removing old text node
            if ( ! ( AnimItemlist[i][1]['frame_' + frame_no]['create'])){
              AnimItemlist[i][0].removeChild(AnimItemlist[i][0].childNodes.lastChild);
            // second append the right text node
            }
            AnimItemlist[i][0].appendChild(AnimItemlist[i][1]['frame_' + frame_no]['text']);
            }
         for (k=0; k< AnimItemlist[i][1]['frame_' + frame_no].length; k++){
         attr = AnimItemlist[i][1]['frame_' + frame_no][k][0];
         if  ( attr != 'time' && attr != 'z' && attr != 'create' )
            AnimItemlist[i][0].setAttribute(
                     attr, AnimItemlist[i][1]['frame_' + frame_no][k][1]);
         }
	 // create is at end because of viewBox/use need that.
         if (AnimItemlist[i][1]['frame_' + frame_no]['create']){
	    root.appendChild(AnimItemlist[i][0]);	
            }
         // z is after create, of course...
         if (AnimItemlist[i][1]['frame_' + frame_no]['z']){
	    set_z(AnimItemlist[i][0], AnimItemlist[i][1]['frame_' + frame_no]['z'][1] );	
            }
       }

    }
}
  
function apply_attr(node, name, value){
    // alert(node.nodeName + " " + name + " " + value ); 
}

function speed_down() {

   if (fps > 1) {
     fps--;
     frame_delay = Math.floor ( 1000 / fps);
     speed_text.value = fps + ' fps';
     }
}

function speed_up() {

   if (fps < 25) {
     fps++;
     frame_delay = Math.floor ( 1000 / fps);
     speed_text.value =  fps + ' fps';
     }
}

function start_animation()
{
  var d = new Date();
  time_base = d.getTime();
  next = time_base;
  rate_text.value = '';
  frames_played = frames_total - step;
  if (! anim_running ) animate();
}

function stop_animation()
{
  if (!anim_running) return;
  clearTimeout(anim_running);
  anim_running = 0;

}

function animate()
{
  var id = root.suspendRedraw(1000);	  

  if (step == frames_total) {
     reinit_animation();
     step = 0;
     frames_played = frames_total;
     }
  else apply_frame(step++);
  root.unsuspendRedraw(id);

  var time = new Date();
  leak = time.getTime() - next;
  next = next + frame_delay;
  anim_running = setTimeout("animate()",frame_delay - leak);
}

function reinit_animation() {
  var k;
  while ( base_stack < root.childNodes.length){
     root.removeChild(root.childNodes[base_stack]);
     }
   var e = new Date();
   var time_actuel=e.getTime();

   var interval = (time_actuel - time_base)/frames_played;

   var rate = Math.round(1000/interval);

   rate_text.value = rate + ' fps'

  while ( base_stack < root.childNodes.length){
     root.removeChild(root.childNodes[base_stack]);
     }

   var f = new Date();
   time_base = f.getTime();
}

	
function set_z(node, index) {
        index = parseInt(index) + base_stack;

        if (node.parentNode.childNodes[index] == node) return;	

	var parent = node.parentNode;
	parent.removeChild(node);

        if (index < base_stack) index = base_stack;
        if (index >= parent.childNodes.length) {
           parent.appendChild(node);
           return;
        }

	parent.insertBefore(node,parent.childNodes[index]);
}

