/* gcompris - nice_target_spot.cpp
 *
 * Copyright (C) 2006 Olaf Ronneberger
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/**************************************************************************
**       Title: create nice target spot
**
**    create nice target spots for puzzle pieces in gcpomris shapegame
**    based on distance transform from Pedro F. Felzenszwalb and 
**    Daniel P. Huttenlocher.
**
**************************************************************************/

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include "dt/pnmfile.h"
#include "dt/imconv.h"
#include "dt/dt.h"

int main(int argc, char **argv) {
  if (argc != 3) {
    fprintf(stderr, "usage: %s input(pbm) output(ppm)\n", argv[0]);
    return 1;
  }

  char *input_name = argv[1];
  char *output_name = argv[2];

  // load input
  image<uchar> *input = loadPBM(input_name);

  // compute dt
  image<float> *out = dt(input);

  // take square roots
  for (int y = 0; y < out->height(); y++) {
    for (int x = 0; x < out->width(); x++) {
      imRef(out, x, y) = sqrt(imRef(out, x, y));
    }
  }

  // find maximum
  int xmax = 0;
  int ymax = 0;
  float maxValue = 0;
  for (int y = 0; y < out->height(); y++) {
    for (int x = 0; x < out->width(); x++) {
      if( imRef(out, x, y) > maxValue)
      {
        xmax = x;
        ymax = y;
        maxValue = imRef(out, x, y);
      }
    }
  }
  
  // draw circle at max position into new image (not touching the border)
  image<rgb>* circleimage = new image<rgb>( input->width(), 
                                            input->height(), false);
  rgb white;
  white.r = 255;
  white.g = 255;
  white.b = 255;
  
  circleimage->init(white);

  float circleRadius = 3.5;
  float borderThickness = 1;
  
  float innerCircleSqrRadius = square(circleRadius);
  float outerCircleSqrRadius = square(circleRadius+borderThickness);
  
  rgb dotColor;
  dotColor.r = 255;
  dotColor.g = 0;
  dotColor.b = 0;

  rgb borderColor;
  borderColor.r = 0;
  borderColor.g = 0;
  borderColor.b = 0;
  

  for (int y = 0; y < out->height(); y++) {
    for (int x = 0; x < out->width(); x++) {
      float currentSqrRadius = square(x - xmax) + square(y - ymax);
      
      if( currentSqrRadius <= outerCircleSqrRadius
          && imRef(out, x, y) > 1)
      {
        if( currentSqrRadius <= innerCircleSqrRadius)
        {
          imRef( circleimage, x, y) = dotColor;
        }
        else
        {
          imRef( circleimage, x, y) = borderColor;
        }
      }
    }
  }

  // save output
  savePPM(circleimage, output_name);

  delete input;
  delete out;
  delete circleimage;
  
}
