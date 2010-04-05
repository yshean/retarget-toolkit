% calculating minimum distance between 2 vectors by rotating them
function distance = RotateMinDistance(vector1, vector2)
    d = size(vector1);
    distance = 10000000;
    for i = 1:d(1)
        temp_vector = RotateVector(vector1, i);
        temp_distance = EuclideanDistance(temp_vector, vector2);
        if(temp_distance < distance)
            distance = temp_distance;
    end
end