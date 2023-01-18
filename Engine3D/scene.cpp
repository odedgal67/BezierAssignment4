#include "scene.h"
#include "glad/include/glad/glad.h"
#include "../Game/Bezier1D.h"
#include <iostream>

	static void printMat(const glm::mat4 mat)
	{
		printf(" matrix: \n");
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
				printf("%f ", mat[j][i]);
			printf("\n");
		}
	}

	Scene::Scene()
	{
		//
		glLineWidth(5);
	
		cameras.push_back(new Camera(60.0f,1.0,0.1f,100.0f));		
		pickedShape = -1;
		depth = 0;
		cameraIndx = 0;
		xold = 0;
		yold = 0;
		isActive = false;
	}

	Scene::Scene(float angle,float relationWH,float near1, float far1)
	{
		//glLineWidth(5);
		cameras.push_back(new Camera(angle,relationWH,near1,far1));
		pickedShape = -1;
		depth = 0;
		cameraIndx = 0;
		xold = 0;
		yold = 0;
		isActive = false;
	}

	void Scene::AddShapeFromFile(const std::string& fileName,int parent,unsigned int mode)
	{
		chainParents.push_back(parent);
		shapes.push_back(new Shape(fileName,mode));
	}

	void Scene::AddShape(int type, int parent,unsigned int mode)
	{
		chainParents.push_back(parent);
		shapes.push_back(new Shape(type,mode));
	}

	void Scene::AddShapeCopy(int indx,int parent,unsigned int mode)
	{
		chainParents.push_back(parent);
		shapes.push_back(new Shape(*shapes[indx],mode));
	}

	void Scene::AddShader(const std::string& fileName)
	{
		shaders.push_back(new Shader(fileName));
	}

	void Scene::AddTexture(const std::string& textureFileName,bool for2D)
	{
		textures.push_back(new Texture(textureFileName));
	}

	void Scene::AddTexture(int width,int height, unsigned char *data)
	{
		textures.push_back(new Texture(width,height,data));
	}

	void Scene::AddCamera(glm::vec3& pos , float fov,float relationWH , float zNear, float zFar)
	{
		cameras.push_back(new Camera(fov,relationWH,zNear,zFar));
		cameras.back()->MyTranslate(pos,0);
	}

	void Scene::Draw(int shaderIndx,int cameraIndx,int buffer,bool toClear,bool debugMode)
	{
		glEnable(GL_DEPTH_TEST);
		glm::mat4 Normal = MakeTrans();
	
		glm::mat4 MVP = cameras[cameraIndx]->GetViewProjection()*glm::inverse(cameras[cameraIndx]->MakeTrans());
		int p = pickedShape;
		if(toClear)
		{
			if(shaderIndx>0)
				Clear(1,1,1,1);
			else
				Clear(0,0,0,0);
		}

		for (unsigned int i=0; i<shapes.size();i++)
		{
			if(shapes[i]->Is2Render())
			{
				glm::mat4 Model = Normal * shapes[i]->MakeTrans();
				
				if(shaderIndx > 0)
				{
					Update(MVP,Model,shapes[i]->GetShader());
					shapes[i]->Draw(shaders,textures,false);	
				}
				else 
				{ //picking
					Update(MVP,Model,0);
					shapes[i]->Draw(shaders,textures,true);
				}
			}
		}
		pickedShape = p;
	}

	void Scene::MoveCamera(int cameraIndx,int type,float amt)
	{
		switch (type)
		{
			case xTranslate:
				cameras[cameraIndx]->MyTranslate(glm::vec3(amt,0,0),0);
			break;
			case yTranslate:
				cameras[cameraIndx]->MyTranslate(glm::vec3(0,amt,0),0);
			break;
			case zTranslate:
				cameras[cameraIndx]->MyTranslate(glm::vec3(0,0,amt),0);
			break;
			case xRotate:
				cameras[cameraIndx]->MyRotate(amt,glm::vec3(1,0,0),0);
			break;
			case yRotate:
				cameras[cameraIndx]->MyRotate(amt,glm::vec3(0,1,0),0);
			break;
			case zRotate:
				cameras[cameraIndx]->MyRotate(amt,glm::vec3(0,0,1),0);
			break;
			default:
				break;
		}
	}

	void Scene::ShapeTransformation(int type,float amt)
	{
		if(glm::abs(amt)>1e-5)
		{
			switch (type)
			{
			case xTranslate:
				shapes[pickedShape]->MyTranslate(glm::vec3(amt,0,0),0);
			break;
			case yTranslate:
				shapes[pickedShape]->MyTranslate(glm::vec3(0,amt,0),0);
			break;
			case zTranslate:
				shapes[pickedShape]->MyTranslate(glm::vec3(0,0,amt),0);
			break;
			case xRotate:
				shapes[pickedShape]->MyRotate(amt,glm::vec3(1,0,0),0);
			break;
			case yRotate:
				shapes[pickedShape]->MyRotate(amt,glm::vec3(0,1,0),0);
			break;
			case zRotate:
				shapes[pickedShape]->MyRotate(amt,glm::vec3(0,0,1),0);
			break;
			default:
				break;
			}
		}

	}

	void Scene::Resize(int width,int height)
	{
	
		cameras[0]->SetProjection(cameras[0]->GetAngle(),(float)width/height);
		glViewport(0,0,width,height);
		std::cout<<cameras[0]->GetRelationWH()<<std::endl;
	}

	float Scene::Picking(int x,int y)
	{
		
		
		return 0;
	}
	//return coordinates in global system for a tip of arm position is local system 
	void Scene::MouseProccessing(int button)
	{
		if(pickedShape == -1)
		{
			if(button == 1 ) //Right mouse click - translation of selected point
			{
                auto bezier = dynamic_cast<Bezier1D*>(shapes[shapes.size()-1]);
                Shape* selectedControlPoint = shapes[selectedPointIndex+1];
                glm::vec4 selectedControlPointPosition = selectedControlPoint->trans[3];
                Shape* rightPoint=NULL;
                Shape* leftPoint=NULL;
                glm::vec4 delta = glm::vec4(-xrel/12.0f, yrel/12.0f,0.0f,0.0f);
                if(selectedPointIndex%3==0) //Not a control point
                {
                    if(selectedPointIndex==0)
                    {
                        rightPoint = shapes[selectedPointIndex+2];
                    }
                    else if(selectedPointIndex == shapes.size()-3)
                    {
                        leftPoint = shapes[selectedPointIndex];
                    }
                    else
                    {
                        rightPoint = shapes[selectedPointIndex+2];
                        leftPoint = shapes[selectedPointIndex];
                    }
                    if(rightPoint != NULL)
                    {
                        rightPoint->MyTranslate(glm::vec3(delta[0], delta[1], delta[2]), 0);
                        bezier->UpdatePoint(rightPoint, selectedPointIndex+1);
                    }
                    if(leftPoint != NULL)
                    {
                        leftPoint->MyTranslate(glm::vec3(delta[0], delta[1], delta[2]), 0);
                        bezier->UpdatePoint(leftPoint, selectedPointIndex-1);
                    }
                }

                selectedControlPoint->MyTranslate(glm::vec3(delta[0], delta[1], delta[2]), 0);

                //Update Bezier according to new shapes values
                bezier->UpdatePoint(selectedControlPoint, selectedPointIndex);
                shapes[shapes.size()-1]->SetBezier1DMesh(bezier->GetLine());
			}
			else //Left mouse click - rotate control point selected
			{
                if(selectedPointIndex%3==0)
                {
                    return;
                }
                glm::vec4 selectedControlPoint = shapes[selectedPointIndex+1]->trans[3];
                glm::vec4 pointToRotateAround;
                Shape* pointForContinuation;
                glm::vec4 pointForContinuationPosition;
                int continuationPointIndex;
                if(selectedPointIndex % 3 == 1) //Get previous point
                {
                    pointToRotateAround = shapes[selectedPointIndex]->trans[3];
                    if(selectedPointIndex > 1)
                    {
                        continuationPointIndex = selectedPointIndex-1;
                        pointForContinuation = shapes[continuationPointIndex];
                        pointForContinuationPosition = pointForContinuation->trans[3];
                    }
                }
                else if(selectedPointIndex % 3 == 2) //Get next point
                {
                    pointToRotateAround = shapes[selectedPointIndex+2]->trans[3];
                    if(selectedPointIndex < shapes.size()-4)
                    {
                        continuationPointIndex = selectedPointIndex+3;
                        pointForContinuation = shapes[continuationPointIndex];
                        pointForContinuationPosition = pointForContinuation->trans[3];
                    }
                }
                else
                {
                    std::cerr << "Shouldn't Happen!!!!!!!!!!!!!!!!!!!!!!";
                }

                //Rotation of point
                shapes[selectedPointIndex+1]->MyTranslate(glm::vec3(pointToRotateAround[0]-selectedControlPoint[0], pointToRotateAround[1]-selectedControlPoint[1], pointToRotateAround[2]- selectedControlPoint[2]), 0);
                glm::vec4 rotation = glm::inverse(shapes[selectedPointIndex+1]->rot) * glm::vec4(0, 0, 1, 0);
                glm::vec3 rotationTrimmed = glm::vec3(rotation[0], rotation[1], rotation[2]);
                shapes[selectedPointIndex+1]->MyRotate(xrel / 2.0f, rotationTrimmed, 0);
                shapes[selectedPointIndex+1]->MyRotate(yrel / 2.0f, rotationTrimmed, 0);
                glm::vec4 ret = shapes[selectedPointIndex+1]->rot * (glm::vec4(selectedControlPoint[0] -pointToRotateAround[0], selectedControlPoint[1] -pointToRotateAround[1], selectedControlPoint[2] -pointToRotateAround[2], 0));
                shapes[selectedPointIndex+1]->MyTranslate(glm::vec3(ret[0], ret[1], ret[2]),0);

                shapes[selectedPointIndex+1]->MyRotate(-xrel/2.0f, glm::vec3(rotation[0], rotation[1], rotation[2]),0);
                shapes[selectedPointIndex+1]->MyRotate(-yrel/2.0f, glm::vec3(rotation[0], rotation[1], rotation[2]),0);

                auto bezier = dynamic_cast<Bezier1D*>(shapes[shapes.size()-1]);

                if(isContinuity && pointForContinuation!=NULL)
                {
                    pointForContinuation->MyTranslate(glm::vec3(pointToRotateAround[0]-pointForContinuationPosition[0], pointToRotateAround[1]-pointForContinuationPosition[1], pointToRotateAround[2]- pointForContinuationPosition[2]), 0);
                    glm::vec4 rotation = glm::inverse(pointForContinuation->rot) * glm::vec4(0, 0, 1, 0);
                    glm::vec3 rotationTrimmed = glm::vec3(rotation[0], rotation[1], rotation[2]);
                    pointForContinuation->MyRotate(xrel / 2.0f, rotationTrimmed, 0);
                    pointForContinuation->MyRotate(yrel / 2.0f, rotationTrimmed, 0);
                    glm::vec4 ret = pointForContinuation->rot * (glm::vec4(pointForContinuationPosition[0] -pointToRotateAround[0], pointForContinuationPosition[1] -pointToRotateAround[1], pointForContinuationPosition[2] -pointToRotateAround[2], 0));
                    pointForContinuation->MyTranslate(glm::vec3(ret[0], ret[1], ret[2]),0);

                    pointForContinuation->MyRotate(-xrel/2.0f, glm::vec3(rotation[0], rotation[1], rotation[2]),0);
                    pointForContinuation->MyRotate(-yrel/2.0f, glm::vec3(rotation[0], rotation[1], rotation[2]),0);

                    bezier->UpdatePoint(pointForContinuation, continuationPointIndex);
                }

                //Update Bezier according to new shapes values
                bezier->UpdatePoint(shapes[selectedPointIndex+1], selectedPointIndex);
                shapes[shapes.size()-1]->SetBezier1DMesh(bezier->GetLine());

			}
		}
	}

	void Scene::ZeroShapesTrans()
	{
		for (unsigned int i = 0; i < shapes.size(); i++)
		{
			shapes[i]->ZeroTrans();
		}
	}

	void Scene::ReadPixel()
	{
		glReadPixels(1,1,1,1,GL_DEPTH_COMPONENT,GL_FLOAT,&depth);
	}

	void Scene::UpdatePosition(float xpos, float ypos)
	{
		xrel = xold - xpos;
		yrel = yold - ypos;
		xold = xpos;
		yold = ypos;
	}

	void Scene::HideShape(int shpIndx)
	{
			shapes[shpIndx]->Hide();
	}

	void Scene::UnhideShape(int shpIndx)
	{
			
			shapes[shpIndx]->Unhide();
	}

	void Scene::Clear(float r, float g, float b, float a)
	{
		glClearColor(r, g, b, a);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	Scene::~Scene(void)
{
	for (Shape* shp : shapes)
		{
			delete shp;
		}
	for (Camera* cam : cameras)
		{
			delete cam;
		}
	for(Shader* sdr: shaders)
		{
			delete sdr;
		}
	for(Texture* tex: textures)
		{
			delete tex;
		}

}

void Scene::TranslateShape(int shape_number, glm::vec3 delta) {
    int temp = pickedShape;
    pickedShape = shape_number;
    MyTranslate(delta,0);
    pickedShape = temp;
}


	 
	