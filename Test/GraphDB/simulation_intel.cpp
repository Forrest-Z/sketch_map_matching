#include "GraphDB.hpp"
#include "GraphPlace.hpp"
#include <boost/filesystem.hpp>
#include <boost/iterator/filter_iterator.hpp>
#include <boost/filesystem/path.hpp>
#include <cstdlib>
#include "Thinker_EVG.hpp"
#include "vodigrex/voronoidiagram/ThinkerVoronoi.hpp"
#include "MultipleLineFollower.hpp"
#include "PlaceExtractorList2Place.hpp"
#include "GraphMatcherNeighbor.hpp"


void ImgProc(cv::Mat& testt){
	cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3,3), cv::Point(-1, -1) );
	int number = 20;
	for(int i = 0 ; i < 2 ; ++i){
		cv::erode(testt, testt, kernel);
	}
	for(int i = 0 ; i < number ; ++i){
		cv::dilate(testt, testt, kernel);
	}
	for(int i = 0 ; i < number ; ++i){
		cv::erode(testt, testt, kernel);
	}
}


AASS::graphmatch::GraphPlace getGraphPlace(AASS::topologicalmap::GraphList& gl_real, cv::Mat& test){

	AASS::graphmatch::PlaceExtractorList2Place extract;
	extract.inputMapIn(test);
	extract.inputGraph(gl_real);
// 	std::cout << "Extracting" << std::endl;
	extract.extract();
	return extract.getGraph();	
}


AASS::topologicalmap::GraphList getGraphListEVG(cv::Mat& testin){
	cv::Mat test;
	testin.copyTo(test);
	
// 	cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3,3), cv::Point(-1, -1) );
// 	int number = 20;
// 	for(int i = 0 ; i < number ; ++i){
// 		cv::dilate(test, test, kernel);
// 	}
// 	for(int i = 0 ; i < number ; ++i){
// 		cv::erode(test, test, kernel);
// 	}
	
// 	ImgProc(test);
	
	cv::threshold(test, test, 240, 255, CV_THRESH_BINARY_INV);
	
	cv::imshow("Input EVG", test);
	cv::waitKey(0);
	
	AASS::Thinker_EVG tv;
	
	tv.setPruning(false);
	
	tv.think(test);
	cv::imshow("graph EVG", tv.getResult());
	
	AASS::topologicalmap::MultipleLineFollower mlf;
	mlf.setD(2);
	mlf.inputMap(tv.getResult());
	mlf.thin();
	std::cout << "DONE MAKING GRAPH" << std::endl;
	return mlf.getGraph(0);
	cv::waitKey(0);
	
}


void getGraphListVoronoi(cv::Mat& testin, AASS::topologicalmap::GraphList& gl){
	

	cv::Mat test;
	testin.copyTo(test);
	
	cv::threshold(test, test, 240, 255, CV_THRESH_BINARY);
	
// 	ImgProc(testt);
	
// 	cv::Mat test =  cv::Scalar::all(255) - testt;
	
	cv::imshow("input voronoi", test);
				
	AASS::vodigrex::ThinkerVoronoi tv;
	tv.think(test);
	cv::Mat res = tv.getResult();
	
	cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3,3), cv::Point(-1, -1) );
	for(int i = 0 ; i < 1 ; ++i){
		cv::dilate(res, res, kernel);
	}
	
	cv::imshow("graph Vronoi", res);
	
	AASS::topologicalmap::MultipleLineFollower mlf;
	mlf.setD(2);
	mlf.inputMap(res);
	mlf.thin();
	std::cout << "DONE MAKING GRAPH" << std::endl;
	gl = mlf.getGraph(0);
	
}



void getGraphsList(cv::Mat& test, cv::Mat test_deux, AASS::topologicalmap::GraphList& gl, AASS::topologicalmap::GraphList& gl_deux, AASS::topologicalmap::GraphList& gl_voro, AASS::topologicalmap::GraphList& gl_deux_voro){

	
	gl.clear();
	gl_deux.clear();
	gl_voro.clear();
	gl_deux_voro.clear();
	
	std::cout << "New File test " << std::endl;	
	
	cv::imshow("MODEL", test);
	cv::imshow("INPUT", test_deux);
		
	gl = getGraphListEVG(test);
	cv::Mat graph_test = cv::Mat::zeros(test.rows , test.cols, CV_8UC3);
	gl.draw(graph_test);
	cv::imshow("evg model graph", graph_test);
	
	getGraphListVoronoi(test, gl_voro);
	cv::Mat graph_test_voro = cv::Mat::zeros(test.rows , test.cols, CV_8UC3);
	gl_voro.scale(2);
	gl_voro.draw(graph_test_voro);
	gl_voro.scale(0.5);
	cv::imshow("voro model graph", graph_test_voro);
	
	std::cout << "Doing the model now" << std::endl;
	
	gl_deux = getGraphListEVG(test_deux);
	cv::Mat graph_test_deux = cv::Mat::zeros(test_deux.rows , test_deux.cols, CV_8UC3);
	gl_deux.draw(graph_test_deux);
	cv::imshow("evg graph", graph_test_deux);
	
	getGraphListVoronoi(test_deux, gl_deux_voro);
	cv::Mat graph_test_deux_voro = cv::Mat::zeros(test_deux.rows , test_deux.cols, CV_8UC3);
	gl_deux_voro.scale(2);
	gl_deux_voro.draw(graph_test_deux_voro);
	gl_deux_voro.scale(0.5);
	cv::imshow("voro graph", graph_test_deux_voro);
		
}



int main(int argc,      // Number of strings in array argv
          char *argv[]){

	std::string path("../Test/GraphDB/Sketches/Intel");
	cv::Mat test_model = cv::imread("../Test/GraphDB/Sketches/Intel/model.png", CV_LOAD_IMAGE_GRAYSCALE);
	
	AASS::graphmatch::GraphPlace gp;
	boost::filesystem::path p(path);
	try{
		while(! boost::filesystem::exists(p)){
			std::cout << "need a valid path toward the slam images" << std::endl;
			std::cin >> path;
			p = path;
		}
		
		if(boost::filesystem::is_directory(p)){
			
			std::vector<boost::filesystem::path> v;
			//Get all files and sort them
			std::copy(boost::filesystem::directory_iterator(p), boost::filesystem::directory_iterator(), std::back_inserter(v));
			std::sort(v.begin(), v.end());
			
			for (std::vector<boost::filesystem::path>::const_iterator it (v.begin()); it != v.end(); it = ++it)
			{
				boost::filesystem::path fn = *it;
				
				
				std::string name = fn.filename().stem().string();
				
				if(name != "model"){
					
					cv::Mat input = cv::imread(fn.string(), CV_LOAD_IMAGE_GRAYSCALE);
					
					AASS::topologicalmap::GraphList gl_model;
					AASS::topologicalmap::GraphList gl;
					
					AASS::topologicalmap::GraphList gl_voro_model;
					AASS::topologicalmap::GraphList gl_voro;
					
					getGraphsList(test_model, input, gl_model, gl, gl_voro_model, gl_voro);
					
					cv::waitKey(0);
					
					AASS::graphmatch::GraphPlace gp_model = getGraphPlace(gl_model, test_model);
					cv::Mat graph_test_model = cv::Mat::zeros(test_model.rows , test_model.cols, CV_8UC3);
					gp_model.draw(graph_test_model);
					cv::imshow("evg gp model", graph_test_model);
					
					AASS::graphmatch::GraphPlace gp = getGraphPlace(gl, input);
					cv::Mat graph_test = cv::Mat::zeros(input.rows , input.cols, CV_8UC3);
					gp.draw(graph_test);
					cv::imshow("evg gp", graph_test);
					
					
					//BUG I DON'T KNOW WHY
					
					AASS::graphmatch::GraphPlace gp_voro = getGraphPlace(gl_voro, input);
					cv::Mat graph_test_voro = cv::Mat::zeros(input.rows , input.cols, CV_8UC3);
					gp_voro.scale(2);
					gp_voro.draw(graph_test_voro);
// 					gp_voro.scale(0.5);
					cv::imshow("voro gp", graph_test_voro);
					
					AASS::graphmatch::GraphPlace gp_voro_model = getGraphPlace(gl_voro_model, test_model);
					cv::Mat graph_test_voro_model = cv::Mat::zeros(test_model.rows , test_model.cols, CV_8UC3);
					gp_voro_model.scale(2);
					gp_voro_model.draw(graph_test_voro_model);
// 					gp_voro_model.scale(0.5);
					cv::imshow("voro gp model", graph_test_voro_model);
					
					cv::waitKey(0);
					
					
					AASS::graphmatch::GraphMatcherNeighbor graphmatch_evg;
					AASS::graphmatch::GraphMatcherNeighbor graphmatch_custom;
				// 	AASS::graphmatch::GraphMatcherClusterFiltered graphmatchold;
					
					//MY THING
					graphmatch_evg.planarEditDistanceAlgorithm(gp, gp_model);
					graphmatch_custom.planarEditDistanceAlgorithm(gp_voro, gp_voro_model);
					
					int rows = 0;
					if(test_model.rows > input.rows){
						rows = test_model.rows;
					}
					else{
						rows = input.rows;
					}
					int cols = 0;
					if(test_model.cols > input.cols){
						cols = test_model.cols;
					}
					else{
						cols = input.cols;
					}
					
					cv::Mat drawing = cv::Mat::zeros(rows , cols, CV_8UC3);
					
					std::deque<	
						AASS::graphmatch::Hypothese
					> hypothesis_final = graphmatch_evg.getResult();
					graphmatch_evg.sort(hypothesis_final);
					hypothesis_final[0].drawHypo(gp, gp_model, drawing, drawing, "ALL FINAL EVG", 1);
					hypothesis_final[0].drawMoved(gp, gp_model, drawing, drawing, "ALL FINAL EVG Moved", 1);
					hypothesis_final[0].drawHypo(gp, gp_model, input, test_model, "ALL FINAL EVG on Map", 1);
					
					std::deque<	
						AASS::graphmatch::Hypothese
					> hypothesis_final_custom = graphmatch_custom.getResult();
					graphmatch_custom.sort(hypothesis_final_custom);
					hypothesis_final_custom[0].drawMoved(gp_voro, gp_voro_model, drawing, drawing, "ALL FINAL CUSTOM Moved", 1);
					hypothesis_final_custom[0].drawHypo(gp_voro, gp_voro_model, drawing, drawing, "ALL FINAL CUSTOM", 1);
					
					while (1){
						cv::waitKey(0);
					}
					
				}
	
				
				
			}
			
			
		}
		
		
		
	}
	catch (const boost::filesystem::filesystem_error& ex)
	{
		std::cout << ex.what() << '\n';
	}
	
}