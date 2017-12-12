import numpy as np
import math

maxf = 2
minf = 0.5

class EtaVELTr:
	def __init__(self, numberOfPixels = 25, minn=0., maxx=1.):
		self.nPixels = numberOfPixels
		self.nCorners = self.nPixels + 1
		self.minEta = minn
		self.maxEta = maxx
		#self.corners = []
		self.edgesX = []
		self.edgesY = []
		self.edgesE = []
		self.edgesF = []
		self.edgesG = []
		self.edgesH = []
		self.counts = []
		self.pOffset = (self.maxEta-self.minEta)/self.nPixels
		
		self.cPosX = []
		self.cPosY = []
		self.zPosX = []
		self.zPosY = []
		
		self.sqSums = []
		self.cIteration = 0
		self.bSqSum = 0
		
		self.initGrid()
		#self.calculatePixelCorners()
		self.update()
		
	def initGrid(self):
		dd = 1 / math.sqrt(2)
		
		#self.corners = [ [self.minEta + x * pOffset, self.minEta + y * pOffset] for y in range(self.nPixels)] for x in range(self.nPixels)
		self.cPosX = [ [self.minEta + x * self.pOffset for x in range(self.nCorners)] for y in range(self.nCorners) ]
		self.cPosY = [ [self.minEta + y * self.pOffset for x in range(self.nCorners)] for y in range(self.nCorners) ]
		self.counts = [ [ [0,0,0,0]  for x in range(self.nPixels) ] for y in range(self.nPixels) ]
		self.edgesX = [ [ 1  for x in range(self.nCorners) ] for y in range(self.nCorners + 1) ]
		self.edgesY = [ [ 1  for x in range(self.nCorners+1) ] for y in range(self.nCorners) ]
		self.edgesE = [ [ dd  for x in range(self.nPixels) ] for y in range(self.nPixels) ]
		self.edgesF = [ [ dd  for x in range(self.nPixels) ] for y in range(self.nPixels) ]
		self.edgesG = [ [ dd  for x in range(self.nPixels) ] for y in range(self.nPixels) ]
		self.edgesH = [ [ dd  for x in range(self.nPixels) ] for y in range(self.nPixels) ]
		
		
		
	def update(self):
		self.normalizeEdgeLengths()
		self.calculateEdgeLengths2()
		self.calculatePixelCorners2()
		conv = False
		out = 0
		outList = []
		tot = self.nPixels*self.nPixels*4
		sqSum = 0
		avg = self.getAvgCounts()
		avgPS = self.getAvgCounts() + 1*math.sqrt(self.getAvgCounts())
		avgMS = self.getAvgCounts() - 1*math.sqrt(self.getAvgCounts())
		for y in range(self.nPixels):
			for x in range(self.nPixels):
				for t in range(4):
					sqSum += (avg -self.counts[y][x][t]) * (avg -self.counts[y][x][t])
					if self.counts[y][x][t] > avgPS or self.counts[y][x][t] < avgMS:
						
						out += 1
						outList.append([y,x,t,self.counts[y][x][t]])
		
		outList = sorted(outList,key=lambda t: abs(self.counts[t[0]][t[1]][t[2]]/self.getAvgCounts()))
		self.counts = [ [ [0,0,0,0]  for x in range(self.nPixels) ] for y in range(self.nPixels) ]
		print("There are {} of {} triangles out of 1 std ({} %)".format(out,tot,out/tot*100))
		print("Total Sq Err: {}".format(sqSum))
		
		self.sqSums.append(sqSum)
		
		if len(self.sqSums) > 2 and np.diff(self.sqSums)[-1] > 0:
			print("converged after {} steps: sqSums {} diff(sqSums) {}".format(self.cIteration,self.sqSums,np.diff(self.sqSums)))
			conv = True
			self.bSqSum = self.sqSums[-2]
					
		self.cIteration += 1
		return [conv,outList,sqSum]
		
	def normalizeEdgeLengths(self):
		sumL = 0
		sumL += sum(map(sum,zip(*self.edgesX)))
		sumL += sum(map(sum,zip(*self.edgesY)))
		sumL += sum(map(sum,zip(*self.edgesE)))
		sumL += sum(map(sum,zip(*self.edgesF)))
		sumL += sum(map(sum,zip(*self.edgesG)))
		sumL += sum(map(sum,zip(*self.edgesH)))
		avgL = sumL/(4*self.nPixels*self.nPixels+2*self.nCorners*(self.nCorners+1))
		print("total Sum is {} avg: {}".format(sumL,avgL))
		self.edgesX = [ [ x/avgL  for x in y ] for y in self.edgesX ]
		self.edgesY = [ [ x/avgL  for x in y ] for y in self.edgesY ]
		self.edgesE = [ [ x/avgL  for x in y ] for y in self.edgesE ]
		self.edgesF = [ [ x/avgL  for x in y ] for y in self.edgesF ]
		self.edgesG = [ [ x/avgL  for x in y ] for y in self.edgesG ]
		self.edgesH = [ [ x/avgL  for x in y ] for y in self.edgesH ]
		
	def _shapeF(self,f):
		f = (f - 1) * 0.6 + 1
		if f > maxf:
			return maxf
		if f < minf:
			return minf
		return f
	
	def calculateEdgeLengths2(self):
		if self.getTotalCounts() == 0:
			return
		avg = self.getAvgCounts()
		
		for y in range(self.nPixels):
			for x in range(self.nPixels):
				for t in range(4):
					pc = self.counts[y][x][t]
					if pc == 0:
						f = maxf
					else:
						f = math.sqrt(avg/pc)
					if pc > avg-math.sqrt(avg) and pc < avg+math.sqrt(avg):
						f = 1.
					sf = self._shapeF(f)
					if t == 0:
						self.edgesX[y][x] = self.edgesX[y][x] / sf
						self.edgesE[y][x] = self.edgesE[y][x] / sf
						self.edgesF[y][x] = self.edgesF[y][x] / sf
					if t == 1:
						self.edgesY[y][x+1] = self.edgesY[y][x+1] / sf
						self.edgesF[y][x] = self.edgesF[y][x] / sf
						self.edgesH[y][x] = self.edgesH[y][x] / sf
					if t == 2:
						self.edgesX[y+1][x] = self.edgesX[y+1][x] / sf
						self.edgesH[y][x] = self.edgesH[y][x] / sf
						self.edgesG[y][x] = self.edgesG[y][x] / sf
					if t == 3:
						self.edgesY[y][x] = self.edgesY[y][x] / sf
						self.edgesG[y][x] = self.edgesG[y][x] / sf
						self.edgesE[y][x] = self.edgesE[y][x] / sf	
			
			
	def calculatePixelCorners2(self):
		w = 20
		posMat = []
		CrVx = np.zeros((self.nCorners,self.nCorners))
		CrVy = np.zeros((self.nCorners,self.nCorners))
		ZrVx = np.zeros((self.nPixels,self.nPixels))
		ZrVy = np.zeros((self.nPixels,self.nPixels))
		
		#boundary conditions matrix/vectors
		BCposMatX = []
		BCposMatY = []
		BCrVx = []
		BCrVy = []
		
		for y in range(self.nCorners):
			for x in range(self.nCorners):
				BClineX = np.zeros((self.nCorners,self.nCorners))
				BClineY = np.zeros((self.nCorners,self.nCorners))
				if 	(x == 0 and y == 0) or \
					(x == 0 and y == self.nPixels) or \
					(x == self.nPixels and y == 0) or \
					(x == self.nPixels and y == self.nPixels):
					BClineX[y][x] = w
					BClineY[y][x] = w
					BCrVx.append(self.getCornerPos(y,x)[0] * w)
					BCrVy.append(self.getCornerPos(y,x)[1] * w)
					#print("bclinex shape {} zeros shape {}".format( BClineX.reshape((self.nCorners*self.nCorners,)).shape , np.zeros((self.nPixels*self.nPixels)).shape ))
					BCposMatX.append(np.hstack((BClineX.reshape((self.nCorners*self.nCorners,)),np.zeros((self.nPixels*self.nPixels,)) )) )
					BCposMatY.append(np.hstack((BClineY.reshape((self.nCorners*self.nCorners,)),np.zeros((self.nPixels*self.nPixels,)) )) )
										
				elif x == 0 or x == self.nPixels:
					BClineX[y][x] = w
					#BClineY[y][x] = 1
					BCrVx.append(self.getCornerPos(y,x)[0] * w)
					#BCrVy.append(self.getCornerPos(y,x)[1])
					BCposMatX.append(np.hstack((BClineX.reshape((self.nCorners*self.nCorners,)),np.zeros((self.nPixels*self.nPixels,)) )) )
					#BCposMatY.append(np.hstack((BClineY.reshape((self.nCorners*self.nCorners,)),np.zeros((self.nPixels*self.nPixels,)) )) )
				elif y == 0 or y == self.nPixels:
					#BClineX[y][x] = 1
					BClineY[y][x] = w
					#BCrVx.append(self.getCornerPos(y,x)[0])
					BCrVy.append(self.getCornerPos(y,x)[1] * w)
					#BCposMatX.append(np.hstack((BClineX.reshape((self.nCorners*self.nCorners,)),np.zeros((self.nPixels*self.nPixels,)) )) )
					BCposMatY.append(np.hstack((BClineY.reshape((self.nCorners*self.nCorners,)),np.zeros((self.nPixels*self.nPixels,)) )) )
				
				
				eLength = 0
				
				if x != 0:
					eLength += self.edgesX[y][x-1]
				if y != 0:
					eLength += self.edgesY[y-1][x]
				if x != self.nPixels:
					eLength += self.edgesX[y][x]
				if y != self.nPixels:
					eLength += self.edgesY[y][x]
					
				if y != 0 and x != 0:
					eLength += self.edgesH[y-1][x-1]
				if y != self.nPixels and x != 0:
					eLength += self.edgesF[y][x-1]
				if y != 0 and x != self.nPixels:
					eLength += self.edgesG[y-1][x]
				if y != self.nPixels and x != self.nPixels:
					eLength += self.edgesE[y][x]
				
				line = np.zeros((self.nCorners,self.nCorners))
				lineZ = np.zeros((self.nPixels,self.nPixels))
				
					
				if x != 0:
					line[y][x-1] = - self.edgesX[y][x-1]/eLength
				if y != 0:
					line[y-1][x] = - self.edgesY[y-1][x]/eLength
				if x != self.nPixels:
					line[y][x+1] = - self.edgesX[y][x]/eLength
				if y != self.nPixels:
					line[y+1][x] = - self.edgesY[y][x]/eLength
					
				if y != 0 and x != 0:
					lineZ[y-1][x-1] = -self.edgesH[y-1][x-1]/eLength
				if y != self.nPixels and x != 0:
					lineZ[y][x-1] = -self.edgesF[y][x-1]/eLength
				if y != 0 and x != self.nPixels:
					lineZ[y-1][x] = -self.edgesG[y-1][x]/eLength
				if y != self.nPixels and x != self.nPixels:
					lineZ[y][x] = -self.edgesE[y][x]/eLength
				
				
				line[y][x] = 1
				CrVx[y][x] = 0
				CrVy[y][x] = 0
				posMat.append( \
					np.hstack(( \
					line.reshape((self.nCorners*self.nCorners,)), \
					lineZ.reshape((self.nPixels*self.nPixels,))	\
					)) \
					)
					
		for y in range(self.nPixels):
			for x in range(self.nPixels):
				line = np.zeros((self.nCorners,self.nCorners))
				lineZ = np.zeros((self.nPixels,self.nPixels))
				
				eLength = self.edgesE[y][x] + self.edgesF[y][x] +self.edgesG[y][x] +self.edgesH[y][x]
				line[y][x] = -self.edgesE[y][x] / eLength
				line[y][x+1] = -self.edgesF[y][x] / eLength
				line[y+1][x] = -self.edgesG[y][x] / eLength
				line[y+1][x+1] = -self.edgesH[y][x] / eLength
				
				lineZ[y][x] = 1
				ZrVx[y][x] = 0
				ZrVy[y][x] = 0
				posMat.append( \
					np.hstack(( \
					line.reshape((self.nCorners*self.nCorners,)), \
					lineZ.reshape((self.nPixels*self.nPixels,))	\
					)) \
					)
				
		CrVxFlat = CrVx.reshape((self.nCorners*self.nCorners,))
		CrVyFlat = CrVy.reshape((self.nCorners*self.nCorners,))
		ZrVxFlat = ZrVx.reshape((self.nPixels*self.nPixels,))
		ZrVyFlat = ZrVy.reshape((self.nPixels*self.nPixels,))
		posMat = np.asarray(posMat)
		
		BCrVyFlat = np.asarray(BCrVy)
		BCrVxFlat = np.asarray(BCrVx)
		BCposMatX = np.asarray(BCposMatX)
		BCposMatY = np.asarray(BCposMatY)
		
		print ("BCposMatY vy {} shape posMat {}".format(BCposMatY.shape,posMat.shape))
		
		FinalrVy = np.hstack((CrVyFlat,ZrVyFlat,BCrVyFlat))
		FinalrVx = np.hstack((CrVxFlat,ZrVxFlat,BCrVxFlat))
		FinalposMatX = np.vstack((posMat,BCposMatX))
		FinalposMatY = np.vstack((posMat,BCposMatY))
		
		print("posMat shape {}".format(posMat.shape))
		print("posMatX shape {}".format(FinalposMatX.shape))
		print("rVxFlat shape {}".format(FinalrVx.shape))
		
		#print("posMat {}".format(FinalposMat))
		#print("rVxFlat {}".format(FinalrVx))
		
		xPos = np.linalg.lstsq(FinalposMatX,FinalrVx)[0]
		yPos = np.linalg.lstsq(FinalposMatY,FinalrVy)[0]

		print("xPosShape {} cutXPosShape {}".format(xPos.shape,xPos[:self.nCorners][:self.nCorners].shape))

		self.cPosX = xPos[:self.nCorners*self.nCorners].reshape((self.nCorners,self.nCorners))
		self.cPosY = yPos[:self.nCorners*self.nCorners].reshape((self.nCorners,self.nCorners))
		
		self.zPosX = xPos[self.nCorners*self.nCorners:].reshape((self.nPixels,self.nPixels))
		self.zPosY = yPos[self.nCorners*self.nCorners:].reshape((self.nPixels,self.nPixels))
					

		
	def fill(self,yy,xx,count = 1):
		[y,x,t] = self.getPixel(yy,xx)
		self.counts[y][x][t] += count
		
	def getCountDist(self):
		c = []
		for y in range(self.nPixels):
			for x in range(self.nPixels):
				c.append(self.counts[y][x])
		return c

	def getPixel(self,yy,xx, debug = False):
		for y in range(self.nPixels):
			for x in range(self.nPixels):
				for t in range(4):
					[v1x,v1y,v2x,v2y,v3x,v3y] = self.getTriangleCorner(y,x,t)
					if self.pointInTriangle([xx,yy],[v1x,v1y],[v2x,v2y],[v3x,v3y]):
						return [y,x,t]
		
		if not debug:	
			raise Exception("not inside a pixel")
		else:
			print("no pixel found")
			return [0,0]
	
	#http://stackoverflow.com/questions/2049582/how-to-determine-a-point-in-a-2d-triangle
	def trSign (self, p1, p2, p3):
		return (p1[0] - p3[0]) * (p2[1] - p3[1]) - (p2[0] - p3[0]) * (p1[1] - p3[1]);

	def pointInTriangle (self,pt, v1, v2, v3):
		b1 = self.trSign(pt, v1, v2) < 0.0
		b2 = self.trSign(pt, v2, v3) < 0.0
		b3 = self.trSign(pt, v3, v1) < 0.0
		return ((b1 == b2) and (b2 == b3));

	def getAvgCounts(self):
		return self.getTotalCounts() / self.nPixels/self.nPixels/4.

	def getTotalCounts(self):
		tot = 0
		for y in range(self.nPixels):
			for x in range(self.nPixels):
				for t in range(4):
					tot += self.counts[y][x][t]
		return tot
		
	#tl tr bl br
	def getPixelCorners(self,iy,ix):
		return self.getCornerPos(iy,ix) + self.getCornerPos(iy,ix+1) + self.getCornerPos(iy+1,ix) + self.getCornerPos(iy+1,ix+1)
		
	def getTriangleCorner(self,iy,ix,tr):
		if tr == 0:
			return self.getCornerPos(iy,ix) + self.getCornerPos(iy,ix+1) + [self.zPosX[iy][ix],self.zPosY[iy][ix]]
		if tr == 1:
			return self.getCornerPos(iy,ix+1) + self.getCornerPos(iy+1,ix+1) + [self.zPosX[iy][ix],self.zPosY[iy][ix]]
		if tr == 2:
			return self.getCornerPos(iy+1,ix+1) + self.getCornerPos(iy+1,ix) + [self.zPosX[iy][ix],self.zPosY[iy][ix]]
		if tr == 3:
			return self.getCornerPos(iy+1,ix) + self.getCornerPos(iy,ix) + [self.zPosX[iy][ix],self.zPosY[iy][ix]]
		
	def getCornerPos(self,iy,ix):
		return [self.cPosX[iy][ix],self.cPosY[iy][ix]]
		
	def getXEdgePos(self,iy,ix):
		p1 = self.getCornerPos(iy,ix)
		p2 = self.getCornerPos(iy,ix+1)
		return [p1[0],p1[1],p2[0],p2[1]]
	
	def getYEdgePos(self,iy,ix):
		p1 = self.getCornerPos(iy,ix)
		p2 = self.getCornerPos(iy+1,ix)
		return [p1[0],p1[1],p2[0],p2[1]]	
		
	def getEEdgePos(self,iy,ix):
		p1 = self.getCornerPos(iy,ix)
		return [p1[0],p1[1],self.zPosX[iy][ix],self.zPosY[iy][ix]]
		
	def getFEdgePos(self,iy,ix):
		p1 = self.getCornerPos(iy,ix+1)
		return [p1[0],p1[1],self.zPosX[iy][ix],self.zPosY[iy][ix]]
		
	def getGEdgePos(self,iy,ix):
		p1 = self.getCornerPos(iy+1,ix)
		return [p1[0],p1[1],self.zPosX[iy][ix],self.zPosY[iy][ix]]
		
	def getHEdgePos(self,iy,ix):
		p1 = self.getCornerPos(iy+1,ix+1)
		return [p1[0],p1[1],self.zPosX[iy][ix],self.zPosY[iy][ix]]
		
