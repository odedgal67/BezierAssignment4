#include "game.h"
#include "Bezier1D.h"
#include <iostream>

#include <glm/gtc/matrix_transform.hpp>

static void printMat(const glm::mat4 mat)
{
	std::cout<<" matrix:"<<std::endl;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
			std::cout<< mat[j][i]<<" ";
		std::cout<<std::endl;
	}
}

Game::Game() : Scene()
{
}

Game::Game(float angle ,float relationWH, float near1, float far1) : Scene(angle,relationWH,near1,far1)
{ 	
}

void Game::Init(int segNum, bool firstTime)
{
    selectedPointIndex = 0;
	AddShader("../res/shaders/pickingShader");	
	AddShader("../res/shaders/basicShader");
	
	AddTexture("../res/textures/box0.bmp",false);

	pickedShape = 0;
    if(firstTime)
    {
        MoveCamera(0,zTranslate,80);
        MoveCamera(0,xTranslate,-15);
    }
	pickedShape = -1;

    CreateBezeir(segNum);

	//ReadPixel(); //uncomment when you are reading from the z-buffer
}

void Game::Update(const glm::mat4 &MVP,const glm::mat4 &Model,const int  shaderIndx)
{
	Shader *s = shaders[shaderIndx];
	int r = ((pickedShape+1) & 0x000000FF) >>  0;
	int g = ((pickedShape+1) & 0x0000FF00) >>  8;
	int b = ((pickedShape+1) & 0x00FF0000) >> 16;
	s->Bind();
	s->SetUniformMat4f("MVP", MVP);
	s->SetUniformMat4f("Normal",Model);
	s->SetUniform4f("lightDirection", 0.0f , 0.0f, -1.0f, 0.0f);
	if(shaderIndx == 0)
		s->SetUniform4f("lightColor",r/255.0f, g/255.0f, b/255.0f,1.0f);
	else 
		s->SetUniform4f("lightColor",0.7f,0.8f,0.1f,1.0f);
	s->Unbind();
}

void Game::WhenRotate()
{
}

void Game::WhenTranslate()
{
}

void Game::Motion()
{
	if(isActive)
	{

        Shape* myCube = shapes[0];
        auto bezier = dynamic_cast<Bezier1D*>(shapes[shapes.size()-1]);

        //Calculate Translation
        auto newPosition = bezier->GetLine().positions[nextCubePosition];
        auto positionsSize = bezier->GetLine().positions.size();
        glm::vec4 newCubePosition = glm::vec4(newPosition[0], newPosition[1], newPosition[2], 0);
        glm::vec4 cubeCurrentPosition = myCube->trans[3];
        glm::vec4 deltaTrans = newCubePosition - cubeCurrentPosition;

        //Calculate Rotation
        glm::vec3 nextNormalVector = bezier->GetLine().normals[nextCubePosition];
        glm::vec3 nextDirectionVector = glm::cross(nextNormalVector,glm::vec3(0,0,1));
        glm::vec3 z = glm::cross(nextDirectionVector,nextNormalVector);


        //Update Cube
        myCube->rot = glm::mat4(1) * glm::inverse(glm::mat4(nextDirectionVector[0], nextNormalVector[0], z[0], 0,
                                                            nextDirectionVector[1], nextNormalVector[1], z[1], 0,
                                                            nextDirectionVector[2], nextNormalVector[2], z[2], 0,
                                    0,   0,   0,   1));
        myCube->MyTranslate(glm::vec3(deltaTrans), 0);


        //Go forward of backward
        if(nextCubePosition>=positionsSize)
        {
            moveForawrd = false;
        }
        if(nextCubePosition<=0)
        {
            moveForawrd = true;
        }
        if(moveForawrd)
        {
            nextCubePosition++;
        }
        else
        {
            nextCubePosition--;
        }

	}
}

Game::~Game(void)
{
}

void Game::CreateBezeir(int segNum)
{
    this->shapes.clear();
    this->bezeirCurve = new Bezier1D(segNum,1441,LINE_STRIP,this, 0); //1441 is (2*3*4*5*6) to match all possible segNums, *2 so improve FPS, + 1
    chainParents.push_back(-1);
    shapes.push_back(bezeirCurve);
}

void Game::MoveSelectedPointRight()
{
    if(selectedPointIndex < shapes.size()-3)
    {
        selectedPointIndex++;
        std::cout << "selected point index is " << selectedPointIndex << "\n";
    }
}

void Game::MoveSelectedPointLeft()
{
    if(selectedPointIndex > 0)
    {
        selectedPointIndex--;
        std::cout << "selected point index is " << selectedPointIndex << "\n";
    }
}
