#include "Bezier1D.h"
#include "scene.h"

Bezier1D::Bezier1D() {}

Bezier1D::Bezier1D(int segNum,int res,int mode, Scene* scene, int viewport)
{
    segmentsNum = segNum;
    resT = res;
    this->setMode(mode);
    this-> all_points = std::vector<glm::vec3>();
    InitScene(scene, segNum);

}

IndexedModel Bezier1D::GetLine() const
{
    IndexedModel model;
    float numberOfPointsPerSegment = (resT - 1) / segmentsNum;

    glm::vec4 p0 = GetControlPoint(0, 0);
    model.positions.emplace_back(glm::vec3(p0[0], p0[1], p0[2]));

    for (int i = 0; i < segmentsNum; i++)
    {
        for (int j = 0; j < numberOfPointsPerSegment; j++) {
            float t = (1.f / numberOfPointsPerSegment) * (j + 1);
            glm::vec4 pOfT = GetPointOnCurve(i, t);
            glm::vec3 dt = GetVelosity(i, t);
            float sqrtT = sqrt(pow(dt.x, 2) + pow(dt.y, 2));
            glm::vec3 normal = glm::vec3(-dt.y / sqrtT, dt.x / sqrtT, 0);

            //Update model positions, colors, normals
            model.positions.emplace_back(glm::vec3(pOfT[0], pOfT[1], pOfT[2]));
            model.colors.emplace_back(glm::vec3(0.f, 1.f, 1.f));
            model.normals.push_back(normal);
        }
    }
    //Update model indices
    for (int i = 0; i < resT; i++)
    {
        model.indices.emplace_back(i);
    }


    return model;
}

glm::vec4 Bezier1D::GetControlPoint(int segment, int indx) const
{
    glm::vec3 ret = all_points[segment*3+indx]*3.f;
    return glm::vec4(ret[0], ret[1], ret[2],0);
}

glm::vec4 Bezier1D::GetPointOnCurve(int segment, float t) const
{
    glm::vec4 b0 = GetControlPoint(segment, 0);
    glm::vec4 b1 = GetControlPoint(segment, 1);
    glm::vec4 b2 = GetControlPoint(segment, 2);
    glm::vec4 b3 = GetControlPoint(segment, 3);

    float a0 = (float)pow(1 - t, 3);
    float a1 = (float)(3 * pow(1 - t, 2) * t);
    float a2 = (float)(3 * (1 - t) * pow(t, 2));
    float a3 = (float)pow(t, 3);

    return a0 * b0 + a1 * b1 + a2 * b2 + a3 * b3;
}

glm::vec3 Bezier1D::GetVelosity(int segment, float t) const
{
    glm::vec4 b0 = GetControlPoint(segment, 0);
    glm::vec4 b1 = GetControlPoint(segment, 1);
    glm::vec4 b2 = GetControlPoint(segment, 2);
    glm::vec4 b3 = GetControlPoint(segment, 3);

    float a0 = -3 * pow(1 - t, 2);
    float a1 = 3 - 12 * t + 9 * pow(t, 2);
    float a2 = 6 * t - 9 * pow(t, 2);
    float a3 = 3 * pow(t, 2);

    glm::vec4 db_t = a0 * b0 + a1 * b1 + a2 * b2 + a3 * b3;
    glm::vec3 res = glm::vec3(db_t[0], db_t[1], db_t[2]);
    return res;
}

void Bezier1D::SplitSegment(int segment, int t)
{
}

void Bezier1D::AddSegment(glm::vec4 p1, glm::vec4 p2, glm::vec4 p3)
{
    glm::vec4 p0 = segments.back()[3];
    segments.push_back(glm::mat4(p0, p1, p2, p3));
}

void Bezier1D::ChangeSegment(int segIndx,glm::vec4 p1, glm::vec4 p2, glm::vec4 p3)
{
    glm::vec4 p0 = segments[segIndx-1][3];
    segments[segIndx] = glm::mat4(p0, p1, p2, p3);
}

float Bezier1D::MoveControlPoint(int segment, int indx, float dx,float dy,bool preserveC1)
{
    return 0; //not suppose to reach here
}

void Bezier1D::CurveUpdate(int pointIndx, float dx, float dy, bool preserveC1)
{
}

void Bezier1D::ResetCurve(int segNum)
{

}

Bezier1D::~Bezier1D(void)
{

}

void Bezier1D::InitScene(Scene *scene, int seg) {
    int num_of_all_points = seg*3 + 1;
    int counter2=0;
    for (int i=0; i<seg; i++)
    {
        if(i==0) //Quarter circle left
        {
            all_points.push_back(glm::vec3(-num_of_all_points,-1.f,0.f));
            all_points.push_back(glm::vec3(-num_of_all_points,1.f,0.f));
            all_points.push_back(glm::vec3(-num_of_all_points + 1,2.2f,0.f));
            all_points.push_back(glm::vec3(-num_of_all_points + 3,2.f,0.f));
            counter2 = 3;
        }
        else if(i==(seg-1)) //Quarter circle right
        {
            all_points.push_back(glm::vec3(all_points[counter2][0] + 2 ,2.2f,0.f));
            counter2++;
            all_points.push_back(glm::vec3(all_points[counter2][0] + 1 ,1.f,0.f));
            counter2++;
            all_points.push_back(glm::vec3(all_points[counter2][0] ,-1.f,0.f));
            counter2++;
        }
        else //Straight Line
        {
            all_points.push_back(glm::vec3( all_points[counter2][0] + 1,2.f,0.f));
            counter2++;
            all_points.push_back(glm::vec3(all_points[counter2][0] + 2 ,2.f,0.f));
            counter2++;
            all_points.push_back(glm::vec3(all_points[counter2][0] + 1,2.f,0.f));
            counter2++;
        }

    }

    //Add Cube
    scene->AddShape(2, -1, 4);

    //Add Points
    int counter = 1;

    for (glm::vec3 pos : all_points)
    {
        pos = pos * 3.0f;
        //pos = pos / glm::vec3(2.0f, 1.f, 1.f);
        //pos = pos - glm::vec3(0,-3,0);
        scene->AddShape(3, -1, 4);
        scene->shapes[counter]->MyTranslate(pos, 0);
        scene->shapes[counter]->MyScale(glm::vec3(0.5, 0.5, 0.5));
        scene->SetShapeTex(counter, 0);
        counter++;
    }


    scene->shapes[0]->MyTranslate(all_points[0]*3.f, 0);
    scene->shapes[0]->MyScale(glm::vec3(0.5, 0.5, 0.5));

    SetBezier1DMesh(GetLine());

}

void Bezier1D::UpdatePoint(Shape* shape, int index) {
    auto point = shape->trans[3];
    all_points[index] = glm::vec3(point[0], point[1], point[2]) / 3.0f;
}

