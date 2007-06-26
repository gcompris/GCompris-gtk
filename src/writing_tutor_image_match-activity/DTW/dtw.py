#   gcompris/tutor/DTW/dtw.py - Writing Tutor project
#
#   Author: Jozef Cerven, 5mi, (c) 2006/2007
#   FACULTY OF MATHEMATICS, PHYSICS AND INFORMATICS COMENIUS UNIVERSITY BRATISLAVA, Informatics Teaching and Basic of Informatics department
#   http://www.edi.fmph.uniba.sk
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


import os
import gtk

from Numeric import *
import math



#
# implementacia DTW algoritmu - na porovnavanie 2och obrazcov(predloha/reference <-> vzorka/sample)
#

# BEGIN OF class DTW ************************************
class DTW:

	def __init__(self, save_path, pattern):
		self.save_path = save_path
		self.pattern = pattern

	def compute_dtw(self, vzorka_file, predloha_file):
		pf = open(predloha_file, "r")
		self.plines = pf.readlines()
		pf.close()

		vf = open(vzorka_file, "r")
		self.vlines = vf.readlines()
		vf.close()


		# matica lokalnych vzdialenosti

		loc_dist =  [None] * len(self.plines)
		glob_dist =  [None] * len(self.plines)
		tmp =  [None] * len(self.plines)

		print "len(self.plines)=", len(self.plines)
		print "predloha_file=", predloha_file
		print "len(self.vlines)=", len(self.vlines)
		print "vzorka_file", vzorka_file

		# zlozitost je O(n*m)
		for i in range(len(self.plines)):
			loc_dist[i] = [None] * len(self.vlines)
			glob_dist[i] = [None] * len(self.vlines)
			tmp[i] = [None] * len(self.vlines)
			pline = self.plines[i].split()

			while gtk.events_pending(): gtk.main_iteration()	# for animation refresh...

			for j in range(len(self.vlines)):
				vline = self.vlines[j].split()
				loc_dist[i][j] = math.sqrt((int(float(pline[0]))-int(float(vline[0])))**2 +
							   (int(float(pline[1]))-int(float(vline[1])))**2 +
							   (int(float(pline[2]))-int(float(vline[2])))**2)




		# matica globalnych vzdialenosti

		i = 0
		j = 0

	        glob_dist[i][j] = loc_dist[i][j]
	        tmp[i][j] = [0,0]  #?

		# j!=0
		for j in range(len(self.vlines)-1):    # 1..n-1
			glob_dist[i][j+1] = loc_dist[i][j+1] + glob_dist[i][j]
			tmp[i][j+1] = [i, j]


		# i!=0
		for i in range(len(self.plines)-1):    # 1..n-1
			j = 0
        		glob_dist[i+1][j] = loc_dist[i+1][j] + glob_dist[i][j]
	            	tmp[i+1][j] = [i,j]

			for j in range(len(self.vlines)-1):    # 1..n-1
				while gtk.events_pending(): gtk.main_iteration()	# for animation refresh...

				glob_dist[i+1][j+1] = loc_dist[i+1][j+1] + min(min(glob_dist[i][j+1],glob_dist[i+1][j]),glob_dist[i][j])
		            	if glob_dist[i][j] <= glob_dist[i][j+1]:
					if glob_dist[i][j] <= glob_dist[i+1][j]:
						tmp[i+1][j+1] = [i,j]
					else:
						tmp[i+1][j+1] = [i+1,j]
				else:
					if glob_dist[i][j+1] <= glob_dist[i+1][j]:
						tmp[i+1][j+1] = [i,j+1]
					else:
						tmp[i+1][j+1] = [i+1,j]



        	return glob_dist[len(glob_dist)-1][len(glob_dist[0])-1]  #?


	# vypocita dtw pre test_vzorka count-krat, teda count-krat porovna test_vzorka s referencnyni predlohami,a vrati najlepsie (min) cislo
	def dtw(self, test_vzorka, count):
		self.count = count
		self.test_vzorka = test_vzorka
		predlohy_dir = self.save_path + "/writing_tutor/predlohy/" + self.pattern + "/"
		if not os.path.exists(predlohy_dir):
			print "%s DOES NOT EXISTS!", predlohy_dir
		else:
			arglist = []
			os.path.walk(predlohy_dir,self.walk_callback,arglist)

		return self.dtw_res


	def walk_callback(self, arg, dirname, fnames ):
		_count = 1
		dtw_res = [None] * 10
		if self.count > 10:
			self.count = 10

		#print "dirname: ", dirname
		if len(fnames) < 1:
			print "dtw.py: walk_callback: len(fnames)<1 !! : ", len(fnames)

		for _filename in fnames:	# porovna test s predlohami
			if os.path.getsize(dirname+_filename) > 0:
				#print "df: ", dirname , _filename

				_dtw_res = self.compute_dtw(self.test_vzorka, dirname + _filename)
				#dtw_res[_count-1] = self.compute_dtw(self.test_vzorka, dirname + _filename)
				dtw_res[_count-1] = int(_dtw_res / ((len(self.plines) * len(self.vlines)))*100)

				print "_dtw_res=", str(_dtw_res),",plines=",str(len(self.plines)),", vlines=",str(len(self.vlines))


				if _count == self.count:	# tolko dtw porovnani, kolko je parameter count
					break
				else:
					_count = _count + 1

		# ak je count > 1, potom vrati minimum z jednotlivych dtw_res
		_min = dtw_res[0]
		for i in range(self.count): # 0..n-1
			if _min > dtw_res[i]:
				_min = dtw_res[i]
			print "dtw_res["+str(i)+"]=", dtw_res[i]
		self.dtw_res = _min


# END OF class DTW ************************************
