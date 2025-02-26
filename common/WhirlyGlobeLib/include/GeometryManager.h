/*  GeometryManager.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 11/25/14.
 *  Copyright 2012-2022 mousebird consulting
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#import <math.h>
#import "WhirlyVector.h"
#import "Scene.h"
#import "SelectionManager.h"
#import "BaseInfo.h"

namespace WhirlyKit
{

typedef enum {GeometryBBoxSingle,GeometryBBoxTriangle,GeometryBBoxNone} GeometryBoundingBox;

// Used to pass geometry around internally
struct GeometryInfo : public BaseInfo
{
    GeometryInfo() = default;
    GeometryInfo(const Dictionary &);
    virtual ~GeometryInfo() = default;

    // Convert contents to a string for debugging
    virtual std::string toString() const override { return BaseInfo::toString() + " +GeomInfo..."; }

    bool colorOverride = false;
    RGBAColor color = RGBAColor::white();
    int boundingBox = GeometryBBoxNone;
    float pointSize = 1.0f;
};
typedef std::shared_ptr<GeometryInfo> GeometryInfoRef;
    
/** The geometry scene representation keeps track of drawables and other
 resources we've created to represent generic geometry passed in by the
 user.
 */
class GeomSceneRep : public Identifiable
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

    GeomSceneRep() : fade(0.0) { }
    GeomSceneRep(SimpleIdentity theID) : Identifiable(theID) { }
    
    // Drawables created for this geometry
    SimpleIDSet drawIDs;
    
    // IDs kept with the selection manager
    SimpleIDSet selectIDs;
    
    // Bounding box (for use in instances of a base model)
    Point3d ll,ur;
    
    // If set, the amount of time to fade out before deletion
    float fade;
    
    // Remove the contents of this scene rep
    void clearContents(SelectionManagerRef &selectManager,ChangeSet &changes,TimeInterval when);
    
    // Enable/disable contents
    void enableContents(SelectionManagerRef &selectManager,bool enable,ChangeSet &changes);
};
    
typedef std::set<GeomSceneRep *,IdentifiableSorter> GeomSceneRepSet;

/// Types supported for raw geometry
typedef enum {WhirlyKitGeometryNone,WhirlyKitGeometryLines,WhirlyKitGeometryTriangles} WhirlyKitGeometryRawType;
    
/// Raw Geometry object.  Fill it in and pass it to the layer.
class GeometryRaw
{
public:
    GeometryRaw();
    GeometryRaw(const GeometryRaw &that);

    /// Simple triangle representation
    class RawTriangle
    {
    public:
        RawTriangle() { }
        /// Construct with three vertex indices
        RawTriangle(int v0,int v1,int v2) { verts[0] = v0; verts[1] = v1; verts[2] = v2; }
        /// Vertices are indices into a vertex array
        int verts[3];
    };
    
    // Compares type and texture ID
    bool operator == (const GeometryRaw &that) const;

    // Runs a consistency check
    bool isValid() const;

    /// Apply the given tranformation matrix to the geometry (and normals)
    void applyTransform(const Eigen::Matrix4d &mat);
    
    // How big the geometry is likely to be in a drawable
    void estimateSize(int &numPts,int &numTris);
    
    // Calculate bounding box
    void calcBounds(Point3d &ll,Point3d &ur);
    
    // Build geometry into a drawable, using the given transform
    void buildDrawables(std::vector<BasicDrawableBuilderRef> &draws,const Eigen::Matrix4d &mat,const RGBAColor *colorOverride,const GeometryInfo *geomInfo,SceneRenderer *sceneRender);

public:
    /// What sort of geometry this is
    WhirlyKitGeometryRawType type;
    /// The points (vertices)
    Point3dVector pts;
    /// Normals to go with the points
    Point3dVector norms;
    /// Texture coordinates, one for each point
    std::vector<WhirlyKit::TexCoord> texCoords;
    /// Colors to go with the points
    std::vector<WhirlyKit::RGBAColor> colors;
    /// The triangles, which reference points
    std::vector<RawTriangle> triangles;
    /// Texture IDs for the geometry
    std::vector<SimpleIdentity> texIDs;
};

/// Represents a single Geometry Instance
class GeometryInstance : public Identifiable
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

    GeometryInstance() : mat(mat.Identity()), colorOverride(false), selectable(false), duration(0.0) { }
    
    // Center for the instance
    Point3d center;
    // End center for the instance
    Point3d endCenter;
    // Duration for the animation
    TimeInterval duration;
    // Rotation etc... for the instance
    Eigen::Matrix4d mat;
    // Set if we're forcing the colors in an instance
    bool colorOverride;
    RGBAColor color;
    // True if this is selectable
    bool selectable;
};
    
// Data types for geometry attributes
typedef enum {GeomRawIntType,GeomRawFloatType,GeomRawFloat2Type,GeomRawFloat3Type,GeomRawFloat4Type,GeomRawDouble2Type,GeomRawDouble3Type,GeomRawTypeMax} GeomRawDataType;

// Geometry attribute base class
class GeomPointAttrData
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

    GeomPointAttrData(GeomRawDataType dataType) : dataType(dataType) { }
    StringIdentity nameID;
    GeomRawDataType dataType;
    virtual int getNumVals() = 0;
    virtual ~GeomPointAttrData() { }
};

// Int geometry attribute
class GeomPointAttrDataInt : public GeomPointAttrData
{
public:
    GeomPointAttrDataInt() : GeomPointAttrData(GeomRawIntType) { }
    int getNumVals() { return (int)vals.size(); }
    virtual ~GeomPointAttrDataInt() { }
    std::vector<int> vals;
};

// Float geometry attribute
class GeomPointAttrDataFloat : public GeomPointAttrData
{
public:
    GeomPointAttrDataFloat() : GeomPointAttrData(GeomRawFloatType) { }
    int getNumVals() { return (int)vals.size(); }
    virtual ~GeomPointAttrDataFloat() { }
    std::vector<float> vals;
};

// Float2 geometry attribute
class GeomPointAttrDataPoint2f : public GeomPointAttrData
{
public:
    GeomPointAttrDataPoint2f() : GeomPointAttrData(GeomRawFloat2Type) { }
    int getNumVals() { return (int)vals.size(); }
    virtual ~GeomPointAttrDataPoint2f() { }
    Point2fVector vals;
};

// Double2 geometry attribute
class GeomPointAttrDataPoint2d : public GeomPointAttrData
{
public:
    GeomPointAttrDataPoint2d() : GeomPointAttrData(GeomRawDouble2Type) { }
    int getNumVals() { return (int)vals.size(); }
    virtual ~GeomPointAttrDataPoint2d() { }
    Point2dVector vals;
};

// Float3 geometry attribute
class GeomPointAttrDataPoint3f : public GeomPointAttrData
{
public:
    GeomPointAttrDataPoint3f() : GeomPointAttrData(GeomRawFloat3Type) { }
    int getNumVals() { return (int)vals.size(); }
    virtual ~GeomPointAttrDataPoint3f() { }
    Point3fVector vals;
};

// Double3 geometry attribute
class GeomPointAttrDataPoint3d : public GeomPointAttrData
{
public:
    GeomPointAttrDataPoint3d() : GeomPointAttrData(GeomRawDouble3Type) { }
    int getNumVals() { return (int)vals.size(); }
    virtual ~GeomPointAttrDataPoint3d() { }
    Point3dVector vals;
};

// Double4 geometry attribute
class GeomPointAttrDataPoint4f : public GeomPointAttrData
{
public:
    GeomPointAttrDataPoint4f() : GeomPointAttrData(GeomRawFloat4Type) { }
    int getNumVals() { return (int)vals.size(); }
    virtual ~GeomPointAttrDataPoint4f() { }
    Vector4fVector vals;
};
    
/// An optimized version of raw geometry for points only
class GeometryRawPoints
{
public:
    GeometryRawPoints();
    ~GeometryRawPoints();
    
    // Check if we've got a consistent set of attributes
    bool valid() const;
    
    // Add an integer to the list of attributes
    void addValue(int idx,int val);
    void addValues(int idx,const std::vector<int> &vals);
    
    // Add a single float to a list of attributes
    void addValue(int idx,float val);
    void addValues(int idx,const std::vector<float> &vals);
    
    // Add two floats to a list of attributes
    void addPoint(int idx,const Point2f &pt);
    void addPoints(int idx,const Point2fVector &pts);
    
    // Add three floats to a list of attributes
    void addPoint(int idx,const Point3f &pt);
    void addPoints(int idx,const Point3fVector &pts);
    
    // Add three doubles to a list of attributes
    void addPoint(int idx,const Point3d &pt);
    void addPoints(int idx,const Point3dVector &pts);
    
    // Add four floats to a list of attributes
    void addPoint(int idx,const Eigen::Vector4f &pt);
    void addPoints(int idx,const Vector4fVector &pts);
    
    // Add an attribute type to the point geometry
    int addAttribute(StringIdentity nameID,GeomRawDataType dataType);
    
    // Find an attribute by name
    int findAttribute(StringIdentity nameID) const;
    
public:
    void buildDrawables(std::vector<BasicDrawableBuilderRef> &draws,const Eigen::Matrix4d &mat,GeometryInfo *geomInfo,SceneRenderer *sceneRender) const;
    
    std::vector<WhirlyKit::GeomPointAttrData *> attrData;
};

#define kWKGeometryManager "WKGeometryManager"
    
/** The Geometry manager displays of simple geometric objects,
 such as spheres, lines, and polygons.
 */
class GeometryManager : public SceneManager
{
public:
    GeometryManager() = default;
    virtual ~GeometryManager();
    
    /// Add raw geometry at the given location
    SimpleIdentity addGeometry(std::vector<GeometryRaw *> &geom,const std::vector<GeometryInstance *> &instances,GeometryInfo &geomInfo,ChangeSet &changes);
    
    /// Add geometry we're planning to reuse (as a model, for example)
    SimpleIdentity addBaseGeometry(std::vector<GeometryRaw *> &geom,const GeometryInfo &geomInfo,ChangeSet &changes);
    SimpleIdentity addBaseGeometry(std::vector<GeometryRaw> &inGeom,const GeometryInfo &geomInfo,ChangeSet &changes);
    
    /// Add instances that reuse base geometry
    SimpleIdentity addGeometryInstances(SimpleIdentity baseGeomID,const std::vector<GeometryInstance> &instances,GeometryInfo &geomInfo,ChangeSet &changes);
    
    /// Add a GPU geometry instance
    SimpleIdentity addGPUGeomInstance(SimpleIdentity baseGeomID,SimpleIdentity programID,SimpleIdentity texSourceID,SimpleIdentity srcProgramID,GeometryInfo &geomInfo,ChangeSet &changes);
    
    /// Add raw geometry points.
    SimpleIdentity addGeometryPoints(const GeometryRawPoints &geomPoints,const Eigen::Matrix4d &mat,GeometryInfo &geomInfo,ChangeSet &changes);

    /// Enable/disable active billboards
    void enableGeometry(SimpleIDSet &billIDs,bool enable,ChangeSet &changes);
    
    /// Remove a group of billboards named by the given ID
    void removeGeometry(SimpleIDSet &billIDs,ChangeSet &changes);
    
    /// Apply the given uniform block to the geometry
    void setUniformBlock(const SimpleIDSet &geomIDs,const RawDataRef &uniBlock,int bufferID,ChangeSet &changes);
    
protected:
    GeomSceneRepSet sceneReps;
};
typedef std::shared_ptr<GeometryManager> GeometryManagerRef;

}
