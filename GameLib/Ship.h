/***************************************************************************************
 * Original Author:     Gabriele Giuseppini
 * Created:             2018-01-21
 * Copyright:           Gabriele Giuseppini  (https://github.com/GabrieleGiuseppini)
 ***************************************************************************************/
#pragma once

#include "GameParameters.h"
#include "GameTypes.h"
#include "MaterialDatabase.h"
#include "Physics.h"
#include "RenderContext.h"
#include "RunningAverage.h"
#include "ShipDefinition.h"
#include "Vectors.h"

#include <optional>
#include <vector>

namespace Physics
{

class Ship
{
public:

    Ship(
        int id,
        World & parentWorld,
        std::shared_ptr<IGameEventHandler> gameEventHandler,
        Points && points,
        Springs && springs,
        Triangles && triangles,
        ElectricalElements && electricalElements,
        VisitSequenceNumber currentVisitSequenceNumber);

    ~Ship();

    unsigned int GetId() const { return mId; }

    World const & GetParentWorld() const { return mParentWorld; }
    World & GetParentWorld() { return mParentWorld; }

    size_t GetPointCount() const { return mPoints.GetElementCount(); }

    auto const & GetPoints() const { return mPoints; }
    auto & GetPoints() { return mPoints; }

    auto const & GetSprings() const { return mSprings; }
    auto & GetSprings() { return mSprings; }

    auto const & GetTriangles() const { return mTriangles; }
    auto & GetTriangles() { return mTriangles; }

    auto const & GetElectricalElements() const { return mElectricalElements; }
    auto & GetElectricalElements() { return mElectricalElements; }

    void DestroyAt(
        vec2f const & targetPos,
        float radius);

    void SawThrough(
        vec2f const & startPos,
        vec2f const & endPos);

    void DrawTo(
        vec2f const & targetPos,
        float strength);

    void SwirlAt(
        vec2f const & targetPos,
        float strength);

    bool TogglePinAt(
        vec2f const & targetPos,
        GameParameters const & gameParameters);

    bool ToggleTimerBombAt(
        vec2f const & targetPos,
        GameParameters const & gameParameters);

    bool ToggleRCBombAt(
        vec2f const & targetPos,
        GameParameters const & gameParameters);

    void DetonateRCBombs();

    ElementIndex GetNearestPointIndexAt(
        vec2f const & targetPos,
        float radius) const;

    void Update(
        VisitSequenceNumber currentVisitSequenceNumber,
        GameParameters const & gameParameters);

    void Render(
        GameParameters const & gameParameters,
        RenderContext & renderContext) const;

public:

    /////////////////////////////////////////////////////////////////////////
    // Dynamics
    /////////////////////////////////////////////////////////////////////////

    // Mechanical

    void UpdateMechanicalDynamics(GameParameters const & gameParameters);

    void UpdateDrawForces(
        vec2f const & position,
        float forceStrength);

    void UpdateSwirlForces(
        vec2f const & position,
        float forceStrength);

    void UpdatePointForces(GameParameters const & gameParameters);

    void UpdateSpringForces(GameParameters const & gameParameters);    

    void IntegratePointForces();

    void HandleCollisionsWithSeaFloor();

    // Water

    void UpdateWaterDynamics(GameParameters const & gameParameters);

    void UpdateWaterInflow(
        GameParameters const & gameParameters,
        float & waterTaken);

    void UpdateWaterVelocities(
        GameParameters const & gameParameters,
        float & waterSplashed);

    // Electrical 

    void UpdateElectricalDynamics(
        VisitSequenceNumber currentVisitSequenceNumber,
        GameParameters const & gameParameters);

    void UpdateElectricalConnectivity(VisitSequenceNumber currentVisitSequenceNumber);

    void DiffuseLight(GameParameters const & gameParameters);

private:

    void DetectConnectedComponents(VisitSequenceNumber currentVisitSequenceNumber);

    void DestroyConnectedTriangles(ElementIndex pointElementIndex);

    void DestroyConnectedTriangles(
        ElementIndex pointAElementIndex,
        ElementIndex pointBElementIndex);

    void BombBlastHandler(
        vec2f const & blastPosition,
        ConnectedComponentId connectedComponentId,
        int blastSequenceNumber,
        int blastSequenceCount,
        GameParameters const & gameParameters);

    void PointDestroyHandler(ElementIndex pointElementIndex);

    void SpringDestroyHandler(
        ElementIndex springElementIndex,
        bool destroyAllTriangles);

    void TriangleDestroyHandler(ElementIndex triangleElementIndex);

    void ElectricalElementDestroyHandler(ElementIndex electricalElementIndex);

private:

    unsigned int const mId;
    World & mParentWorld;
    std::shared_ptr<IGameEventHandler> mGameEventHandler;

    // All the ship elements - never removed, the repositories maintain their own size forever
    Points mPoints;
    Springs mSprings;
    Triangles mTriangles;
    ElectricalElements mElectricalElements;

    // Connected components metadata
    std::vector<std::size_t> mConnectedComponentSizes;

    // Flag remembering whether points (elements) and/or springs (incl. ropes) and/or triangles have changed
    // since the last step.
    // When this flag is set, we'll re-detect connected components and re-upload elements
    // to the rendering context
    bool mutable mAreElementsDirty;

    // Sinking detection
    bool mIsSinking;

    // Water
    float mTotalWater;
    RunningAverage<30> mWaterSplashedRunningAverage;

    // Pinned points
    PinnedPoints mPinnedPoints;

    // Bombs
    Bombs mBombs;


    //
    // Tool force to apply at next iteration
    //

    struct ToolForce
    {
        vec2f Position;
        float Strength;
        bool IsRadial;

        ToolForce(
            vec2f position,
            float strength,
            bool isRadial)
            : Position(position)
            , Strength(strength)
            , IsRadial(isRadial)
        {}
    };

    std::optional<ToolForce> mCurrentToolForce;
};

}