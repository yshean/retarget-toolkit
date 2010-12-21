function myEnergy = GetEnergy(curFMapVector)

curFMapVector = abs(curFMapVector);
curFramePDF = sum(curFMapVector, 2);
wVector = calcICL(curFramePDF);
wVector = repmat(wVector, 1, size(curFMapVector, 2));
myEnergyMat = wVector .* curFMapVector;
myEnergy = sum(myEnergyMat);