"""
Test script to demonstrate the new global triangle ID functionality in Mitsuba3

This script shows how to:
1. Load a scene with multiple meshes
2. Get the total triangle count across all meshes
3. Ray trace and retrieve the global triangle ID from surface interactions
"""

import mitsuba as mi
import drjit as dr

# Set the variant (you can change this to your preferred variant)
mi.set_variant('cuda_ad_rgb')

# Example: Create a simple scene with multiple meshes programmatically
scene_dict = {
    'type': 'scene',
    
    # First mesh: a quad (2 triangles)
    'quad': {
        'type': 'obj',
        'filename': 'path/to/your/quad.obj',  # Replace with actual path
        'bsdf': {
            'type': 'diffuse',
            'reflectance': {'type': 'rgb', 'value': [0.8, 0.2, 0.2]}
        }
    },
    
    # Second mesh: another object
    'object': {
        'type': 'obj', 
        'filename': 'path/to/your/object.obj',  # Replace with actual path
        'bsdf': {
            'type': 'diffuse',
            'reflectance': {'type': 'rgb', 'value': [0.2, 0.8, 0.2]}
        }
    },
    
    # Add a sensor
    'sensor': {
        'type': 'perspective',
        'to_world': mi.ScalarTransform4f.look_at(
            origin=[0, 0, 5],
            target=[0, 0, 0],
            up=[0, 1, 0]
        ),
        'fov': 45,
        'film': {
            'type': 'hdrfilm',
            'width': 512,
            'height': 512,
        }
    },
    
    # Add a light source
    'light': {
        'type': 'constant',
        'radiance': {'type': 'rgb', 'value': 1.0}
    }
}

# Load the scene (or use mi.load_file('path/to/scene.xml'))
# scene = mi.load_dict(scene_dict)

# For demonstration, let's assume you have a scene file:
# scene = mi.load_file('path/to/your/scene.xml')

# Example usage (uncomment when you have a real scene):
"""
# 1. Get total triangle count
total_triangles = scene.total_triangle_count()
print(f"Total triangles in scene: {total_triangles}")

# 2. Print triangle count for each shape
print("\nTriangle count per mesh:")
for i, shape in enumerate(scene.shapes()):
    if hasattr(shape, 'face_count'):  # Check if it's a mesh
        offset = shape.global_triangle_offset()
        count = shape.face_count()
        print(f"  Mesh {i}: {count} triangles (global IDs: {offset} to {offset + count - 1})")

# 3. Ray tracing example - SINGLE RAY
ray = mi.Ray3f(o=[0, 0, 5], d=[0, 0, -1])
si = scene.ray_intersect(ray)

# For scalar/single ray, check if valid first
if dr.all(si.is_valid()):
    # Convert Dr.Jit arrays to Python values for printing
    local_idx = int(si.prim_index) if hasattr(si.prim_index, '__int__') else si.prim_index
    global_idx = int(si.global_prim_index) if hasattr(si.global_prim_index, '__int__') else si.global_prim_index
    
    print(f"\nRay intersection:")
    print(f"  Local primitive index: {local_idx}")
    print(f"  Global primitive index: {global_idx}")
    print(f"  Position: {si.p}")
    print(f"  UV: {si.uv}")
else:
    print("\nNo intersection found")

# 4. Batch ray tracing example
rays = mi.Ray3f(
    o=[[0, 0, 5], [1, 0, 5], [-1, 0, 5]],
    d=[[0, 0, -1], [0, 0, -1], [0, 0, -1]]
)
sis = scene.ray_intersect(rays)

print(f"\nBatch ray intersections:")
print(f"  Valid: {sis.is_valid()}")
# For arrays, you can access individual elements or convert to numpy
print(f"  Global primitive indices: {sis.global_prim_index}")
# Or access specific indices:
# print(f"  First ray global ID: {int(sis.global_prim_index[0])}")
"""

print(__doc__)
print("\nTo use this functionality:")
print("1. Load your scene: scene = mi.load_file('your_scene.xml')")
print("2. Get total triangles: scene.total_triangle_count()")
print("3. Trace rays and access si.global_prim_index")
