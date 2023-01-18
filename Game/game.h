#pragma once
#include "scene.h"
#include "Bezier1D.h"

class Game : public Scene
{
public:
	
	Game();
	Game(float angle,float relationWH,float near, float far);
	void Init(int segNum, bool firstTime);
	void Update(const glm::mat4 &MVP,const glm::mat4 &Model,const int  shaderIndx);
	void ControlPointUpdate();
	void WhenRotate();
	void WhenTranslate();
	void Motion();
	~Game(void);

    void CreateBezeir(int segNum);
    Bezier1D* bezeirCurve;
    int nextCubePosition = 1;
    bool moveForawrd = true;

    void MoveSelectedPointRight();

    void MoveSelectedPointLeft();
};

