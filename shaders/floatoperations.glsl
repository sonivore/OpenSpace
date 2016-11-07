/**
 * Convert a positive floating point distance [0, 10^27]
 * (size of observable universe)
 * to a float in the range [-1, 1], suitable for depth buffer storage.
 * Note: This needs to be a monotonic function, so that the value can
 * still be used for depth comparison.
 */
float normalizeFloat(float inpt) {
    if (inpt > 1.0) {
        return inpt / pow(10, 27);
    } else {
        return inpt - 1.0;
    }
}

float denormalizeFloat(float inpt) {
    if (inpt < 0.0) {
        return inpt + 1.0;
    } else {
        return inpt * pow(10, 27);
    }
}

/**
 * Set z component to zero.
 */
vec4 zNormalize(vec4 vector) {
    return vec4(vector.xy, 0.0, vector.w)
}

/**
 * Compute the length of a vector.
 * Supporting huge vectors, where the square of any of the components is too large to represent as a float. 
 */
float safeLength(vec4 v) {
    vec4 absV = abs(v);
    float m = max(max(max(absV.x, absV.y), absV.z), absV.w);
    if (m > 0.0) {
        return length(v / m) * m;
    } else {
        return 0;
    }
}

float safeLength(vec3 v) {
    vec3 absV = abs(v);
    float m = max(max(absV.x, absV.y), absV.z);
    if (m > 0.0) {
        return length(v / m) * m;
    } else {
        return 0;
    }
}

float safeLength(vec2 v) {
    vec2 absV = abs(v);
    float m = max(absV.x, absV.y);
    if (m > 0.0) {
        return length(v / m) * m;
    } else {
        return 0;
    }
}

/**
 * Normalize a vector
 * Supporting huge vectors, where the square of any of the components is too large to represent as a float. 
 */
vec3 safeNormalize(vec3 v) {
    vec3 absV = abs(v);
    float m = max(max(absV.x, absV.y), absV.z);
    return normalize(v / m);
}
