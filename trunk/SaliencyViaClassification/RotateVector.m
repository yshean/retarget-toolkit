function vector = RotateVector(input_vector, rotate_amount)
    d = size(input_vector);
    vector = ones(d(1), 1);
    vector(rotate_amount + 1:d) = input_vector(1:d - rotate_amount);
    vector(1:rotate_amount) = input_vector(d - rotate_amount + 1:d);
end