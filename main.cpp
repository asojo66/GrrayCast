#include <iostream>
#include <fstream>
#include <cmath>
#define M_PI 3.14159265358979323846

// Vector4D struct for basic vector operations
struct Vec4 {
    float t, x, y, z;

    Vec4 operator+(const Vec4& v) const { return {t + v.t, x + v.x, y + v.y, z + v.z}; }
    Vec4 operator-(const Vec4& v) const { return {t - v.t, x - v.x, y - v.y, z - v.z}; }
    Vec4 operator*(float s) const { return {t * s, x * s, y * s, z * s}; }
    Vec4 normalize() const {
        float len = std::sqrt(t * t + x * x + y * y + z * z);
        return {t / len, x / len, y / len, z / len};
    }
    float dot(const Vec4& v) const { return t * v.t + x * v.x + y * v.y + z * v.z; }
};

// Sphere signed distance function in 4D
float sphereSDF(const Vec4& p, float radius) {
    return std::sqrt(p.t * p.t + p.x * p.x + p.y * p.y + p.z * p.z) - radius;
}

// Ray marching function in 4D
float rayMarch(const Vec4& ro, const Vec4& rd) {
    float depth = 0.0;
    for (int i = 0; i < 100; ++i) { // Max steps
        Vec4 p = ro + rd * depth;
        float dist = sphereSDF(p, 1.0); // Sphere radius = 1.0
        if (dist < 0.001) return depth; // Hit
        depth += dist;
        if (depth > 100.0) break; // Max depth
    }
    return -1.0; // No hit
}

// Estimate normal at point p using central differences
Vec4 estimateNormal(const Vec4& p) {
    const float eps = 0.001f;
    float dt = sphereSDF(Vec4{p.t + eps, p.x, p.y, p.z}, 1.0f) - sphereSDF(Vec4{p.t - eps, p.x, p.y, p.z}, 1.0f);
    float dx = sphereSDF(Vec4{p.t, p.x + eps, p.y, p.z}, 1.0f) - sphereSDF(Vec4{p.t, p.x - eps, p.y, p.z}, 1.0f);
    float dy = sphereSDF(Vec4{p.t, p.x, p.y + eps, p.z}, 1.0f) - sphereSDF(Vec4{p.t, p.x, p.y - eps, p.z}, 1.0f);
    float dz = sphereSDF(Vec4{p.t, p.x, p.y, p.z + eps}, 1.0f) - sphereSDF(Vec4{p.t, p.x, p.y, p.z - eps}, 1.0f);
    Vec4 n = {dt, dx, dy, dz};
    return n.normalize();
}

// Main function
int main() {
    const int width = 800, height = 600;
    std::ofstream image("output.ppm");
    image << "P3\n" << width << " " << height << "\n255\n";

    Vec4 cameraPos = {0, 0, 0, -5}; // Camera position in 4D

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float u = (x - width / 2.0f) / height;
            float v = (y - height / 2.0f) / height;
            Vec4 rayDir = Vec4{0, u, v, 1}.normalize(); // Ray direction in 4D

            float dist = rayMarch(cameraPos, rayDir);
            if (dist > 0) {
                Vec4 hitPoint = cameraPos + rayDir * dist;
                Vec4 normal = estimateNormal(hitPoint);

                // Light direction: from hit point to camera
                Vec4 lightDir = (cameraPos - hitPoint).normalize();

                float diffuse = std::max(0.0f, normal.dot(lightDir));
                float ambient = 0.15f; // Ambient term for visibility

                // --- Checkerboard UV texture ---
                // Project hitPoint onto sphere surface (ignore t for UV)
                float r = std::sqrt(hitPoint.x * hitPoint.x + hitPoint.y * hitPoint.y + hitPoint.z * hitPoint.z);
                float sphere_u = 0.5f + std::atan2(hitPoint.y, hitPoint.x) / (2.0f * M_PI);
                float sphere_v = 0.5f - std::asin(hitPoint.z / r) / M_PI;
                int N = 8; // Checker size
                int checker = ((int(sphere_u * N) + int(sphere_v * N)) % 2);

                int baseColor = checker ? 220 : 40; // Light/dark checker color
                int color = static_cast<int>(baseColor * std::min(1.0f, diffuse + ambient));
                image << color << " " << color << " " << color << "\n";
            } else {
                image << "0 0 0\n"; // Background color
            }
        }
    }

    image.close();
    std::cout << "Image saved as output.ppm\n";
    return 0;
}