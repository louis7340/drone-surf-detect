#include <stdio.h>
#include <iostream>
#include<cstring>
#include <cmath>
#include <dirent.h>
#include<cstdlib>
#include <string>
#include <unistd.h>
#include<map>
#include <opencv2/core/core.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/nonfree/nonfree.hpp>
#include <unistd.h>


using namespace cv;
using namespace std;

//record png file name
map <string,int> fe;

#define x_eps 200
#define y_eps 100
#define good_ma 11
#define k_points 1000


// execute stop command
void stopCommand(char curlcmd [])
{
  usleep(500000); //us
  // sleep(1);
  strcpy(curlcmd,"curl http://localhost:3001/terminal/stop/0.1");
  system(curlcmd);
}


//List files and find new png file under  this directory
string List(char *path)
{
  struct dirent *ent = NULL;
  DIR *pDir;
  char p[5];
  int i;
  char name[150];
	
  //whether there exists such file
  if((pDir = opendir(path)) != NULL)
  {

    while(NULL != (ent = readdir(pDir)))
    {
      int l = strlen(ent->d_name);
      int j = 0;
      for(i=l-4;i<l;i++)
        p[j++] = ent->d_name[i];
      p[4] = '\0';

	  //check for "png" file type
      if(strcmp(p,".png")==0)
      {
        strcpy(name,ent->d_name);
        string x(name);
        
		if(fe[x]==0)  return x;

      }
    }

    closedir(pDir);
  }

  return "no";
}


//store the info about the current command
char curlcmd[150];

int main()
{
  fe.clear();
  fe["done.png"] = 1;
  system("rm -rf ../../tmp/*");	

  //the object file path you should configure
  Mat object = imread( "../sample.jpg", CV_LOAD_IMAGE_GRAYSCALE );
	
  //check 
  if( !object.data )
  {
    std::cout<< "Error reading object " << std::endl;
    return -1;
  }

  //Detect the keypoints using SURF Detector
  int minHessian = k_points;

  SurfFeatureDetector detector( minHessian );
  std::vector<KeyPoint> kp_object;	//used for storing the keypoints

	//detec keypoint and sotre
  detector.detect( object, kp_object );

  //Calculate descriptors (feature vectors)
  SurfDescriptorExtractor extractor;
  Mat des_object;

  //compute keypoints vector
  extractor.compute( object, kp_object, des_object );


  FlannBasedMatcher matcher;


  std::vector<Point2f> obj_corners(4);

  //Get the corners from the object
  obj_corners[0] = cvPoint(0,0);
  obj_corners[1] = cvPoint( object.cols, 0 );
  obj_corners[2] = cvPoint( object.cols, object.rows );
  obj_corners[3] = cvPoint( 0, object.rows );

  // Create capture device ready
  // here 0 indicates that we want to use camera at 0th index

  char key = 'a';
  char ch;
  int framecount = 0;
  int count = 0;

  printf("Tracking!!");
 	//press 't' to start tracking
  while(key!='t' && key!='T') 
    scanf("%c",&key);
  
  printf("OK!");
  while (key != 'e')
  {
    Mat frame;

    //以當目錄為例
    string tmpimg  = List("../../tmp");
    char tmp[50];
    strcpy(tmp,"../../tmp/");
    
	//if find the png file ,copy file name
	if(tmpimg!="no")
    { 

      int i;
      cout<<tmpimg<<endl;
      char tmpname[150];
      for(i=0;i<tmpimg.length();i++)
        tmpname[i] = tmpimg[i];
      tmpname[i] = '\0';

      strcat(tmp,tmpname);
      frame = imread(tmp);
      imshow("tmp image",frame);
      printf("%s\n",tmp);
      rename(tmp,"../../tmp/done.png");
    }
    else 
    {
    	continue ;
    }
    printf("%d\n", count++);
    
    waitKey(100);

	/*
	   same principle as previos ( keypoints....etc)
	  */

    Mat des_image, img_matches;
    std::vector<KeyPoint> kp_image;
    std::vector<vector<DMatch > > matches;
    std::vector<DMatch > good_matches;
    std::vector<Point2f> obj;
    std::vector<Point2f> scene;
    std::vector<Point2f> scene_corners(4);
    Mat H;
    Mat image;

	//convert rgb to gray level
    cvtColor(frame, image, CV_RGB2GRAY);

    detector.detect( image, kp_image );
    extractor.compute( image, kp_image, des_image );

	//match des_object and des_image )
    matcher.knnMatch(des_object, des_image, matches, 1);

	//decide the good match key points 
    for(int i = 0; i < min(des_image.rows-1,(int) matches.size())&&good_matches.size()<=10; i++) //THIS LOOP IS SENSITIVE TO SEGFAULTS
    {
      if((matches[i][0].distance < 0.6*(matches[i][1].distance)) && ((int) matches[i].size()<=2 && (int) matches[i].size()>0))
      {
        good_matches.push_back(matches[i][0]);
      }
    }

    //Draw only "good" matches
    drawMatches( object, kp_object, image, kp_image, good_matches, img_matches, Scalar::all(-1), Scalar::all(-1), vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS );

    if (good_matches.size() >= good_ma)
    {
      strcpy( curlcmd,"curl http://localhost:3001/terminal/");
      for( int i = 0; i < good_matches.size(); i++ )
      {
        //Get the keypoints from the good matches
        obj.push_back( kp_object[ good_matches[i].queryIdx ].pt );
        scene.push_back( kp_image[ good_matches[i].trainIdx ].pt );
      }

      H = findHomography( obj, scene, CV_RANSAC );

      perspectiveTransform( obj_corners, scene_corners, H);

      //Object center
      Point2f ocenter;
      ocenter.x = scene_corners[0].x+scene_corners[1].x+scene_corners[2].x+scene_corners[3].x;
      ocenter.x/=4;
      ocenter.x+= Point2f( object.cols, 0).x;
      ocenter.y = scene_corners[0].y+scene_corners[1].y+scene_corners[2].y+scene_corners[3].y;
      ocenter.y/=4;
      //Draw Circle of Object center
      circle(img_matches,ocenter,20,Scalar( 0, 255, 0), 4);


      //The Center of the window
      Point2f center;
      center.x =( img_matches.cols+object.cols)/2;
      center.y = img_matches.rows/2;
      circle(img_matches,center,20,Scalar( 0,0, 255), 4);

      printf("%f\n", abs(ocenter.x-center.x));
      // if the x-diff excceeds 50
      if(abs(ocenter.x-center.x)>x_eps)
      {
        if(ocenter.x<center.x)
        {
          //Function to Left
          strcpy( curlcmd,"curl http://localhost:3001/terminal/");
          printf("Left\n");
          strcat( curlcmd,"counterClockwise/0.3");
          system(curlcmd);
        }
        else
        {
          //Function to Right
          strcpy( curlcmd,"curl http://localhost:3001/terminal/");
          printf("Right\n");
          strcat( curlcmd,"clockwise/0.3");
          system(curlcmd);
        }
        // printf("%d\n",ocenter.x-center.x);
        stopCommand(curlcmd);
        continue;
      }

      /*
        handle for "front" , "back"
       */
      
      //calculate area
      float x = ( scene_corners[1].x- scene_corners[1].x+scene_corners[2].x- scene_corners[3].x)/2;
      float y = (scene_corners[2].y- scene_corners[1].y+scene_corners[3].y- scene_corners[0].y)/2;
      printf("x * y = %f\n" ,x*y);
      printf("x = %f\n", x);
      printf("y = %f\n", y);
      double area = abs(x * y);
      if(area < 400)
      {
        //front
        strcpy( curlcmd,"curl http://localhost:3001/terminal/");
        printf("Front\n");
        strcat( curlcmd,"front/0.1");
        system(curlcmd);
        stopCommand(curlcmd);
        continue;
      }
      else if (area > 3500)
      {
        //back
        strcpy( curlcmd,"curl http://localhost:3001/terminal/");
        printf("Back\n");
        strcat( curlcmd,"back/0.1");
        system(curlcmd);
        stopCommand(curlcmd);
        continue;
      }

      /*
         handle for "up" , "down"
      */

      //if the y-diff excceeds 50
      if(abs(ocenter.y-center.y)>y_eps)
      {
        if(ocenter.y<center.y)
        {
          //Function to Up
          strcpy( curlcmd,"curl http://localhost:3001/terminal/");
          printf("Up\n");
          strcat( curlcmd,"up/0.3");
          system(curlcmd);
        }
        else
        {
          //Function to Down
          strcpy( curlcmd,"curl http://localhost:3001/terminal/");
          printf("Down\n");
          strcat( curlcmd,"down/0.3");
          system(curlcmd);
        }
        stopCommand(curlcmd);
        continue;
      }
      
      
      
    }

  }

  return 0;
}
