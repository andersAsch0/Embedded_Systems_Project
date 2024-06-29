#include "ppm.h" //this was made by Solarian Programmer (sol_prog) for "Perlin Noise": https://github.com/sol-prog/Perlin_Noise
#include <iostream>
#include <fstream>
// using namespace std;
using std::vector; using std::string; using std::cout; using std::endl; using std::ofstream;

/*
This is the program that I created and used for my CS120B 2024 final project. 
(This is not the exact program I used, it has been significantly cleaned up but functionally is the same). 
It takes a ppm file and converts it into a txt file in the format of a c array of ints according to a set color palette.

You must set the const variables below before running the program.
*/

/// @brief Do not include the file extension. This must be a ppm file in the same directory as this script
const string INPUTFILENAME = "";
/// @brief Do not include the file extension. This will be output as a .txt file in the same directory as this script
const string OUTPUTFILENAME = "ppmReaderOutput";
/// @brief each row corresponds to one number that will be output. Every sRGB color value listed will be mapped to its row index if encountered in the input ppm
const vector<vector<vector<int>>> COLORMAP = { //overwrite these example colors with your own
    {{0,0,0}}, //black
    {{255,0,0}, {252, 0, 0}, {229, 0, 0}}, //red //there is some sort of bug here where on my monitor the sprite is displayed as 255, 1, 1 but ppm is reading 255, 0, 0. idk which is right
    {{236, 204, 66}, {236, 205, 67},{239, 190, 63}}, //yellow
    {{231, 128, 38}, {228, 140, 42}} //orange
};
/// @brief the number that will be output if no color in the colormap matches
const int DEFAULTOUTPUT = 9; 



bool invalidColorErrorFlag = 0;
bool defaultColorFlag = 0;
ppm image(INPUTFILENAME + ".ppm");

int mapPixel(int pixelLocation){
    for(int row = 0; row < COLORMAP.size(); ++row){
        for(int col = 0; col < COLORMAP[row].size(); ++col){
            if(COLORMAP[row][col].size() != 3){
                if (!invalidColorErrorFlag) {
                    cout << "Warning: color row " << row << " col " << col << " in COLORMAP is not initialized correctly. It has been skipped. Each color should have 3 int values." << endl;
                    invalidColorErrorFlag = true;
                }
                continue;
            }
            if((image.r[pixelLocation] == COLORMAP[row][col][0]) && (image.g[pixelLocation] == COLORMAP[row][col][1]) && (image.b[pixelLocation] == COLORMAP[row][col][2])) return row;
        }
    }
    defaultColorFlag = 1;
    return DEFAULTOUTPUT;
}

int main()
{

    ofstream myoutputFile;
    myoutputFile.open(OUTPUTFILENAME + ".txt");
    if(!myoutputFile.is_open()){
        cout << "Error: could not open or create output file." << endl;
        return 0;
    }
    cout << "Total Pixels: " << (int)image.size << endl;
    cout << "Height: " << (int)image.height << endl;
    cout << "Width: " << (int)image.width << endl;   
    for(int i = 0; i < image.height; i++){
            myoutputFile << "{";
        for (int j = 0; j < image.width; j++){
            myoutputFile << mapPixel(i*image.width + j);
            if(j != image.width-1) myoutputFile << ",";

        }
        myoutputFile << "}";
       if((i != image.height-1)) myoutputFile << ",";
        myoutputFile << endl;

    }
    myoutputFile.close();
    if(defaultColorFlag) cout << "Notice: Some pixels were not matched and default color has been used." << endl;

    return 0;
}
