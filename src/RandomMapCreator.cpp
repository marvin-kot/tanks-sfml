#include "RandomMapCreator.h"
#include "EagleController.h"
#include "GameObject.h"
#include "Controller.h"
#include "Shootable.h"
#include "Damageable.h"
#include "ObjectsPool.h"
#include "GlobalConst.h"
#include "SpriteRenderer.h"
#include "Utils.h"

#include <random>


RandomMapCreator::RandomMapCreator() : MapCreator() {}

int RandomMapCreator::parseMapFile(std::string fileName)
{
    // file format:
    // line 1: width, height
    // next lines: map

    std::ifstream f(fileName);
    char cstr[256];

    strcpy(cstr, fileName.c_str());
    std::string line;
    // line 0: map name
    {
        std::getline(f, _currLevelProperies.name);
        assert(_currLevelProperies.name.length() > 0 && _currLevelProperies.name.length() < 100);
    }

    // line 1: win conditions
    {
        std::getline(f, line);
        std::istringstream iss(line);
        std::string cond;
        int  par;

        iss >> cond >> par;
        assert(Level::winMeaningsMap.find(cond) != Level::winMeaningsMap.end());
        _currLevelProperies.win = Level::winMeaningsMap.at(cond);
        _currLevelProperies.winParam = par;
    }
    // line 2: fail conditions
    {
        std::getline(f, line);
        std::istringstream iss(line);
        std::string cond;
        int  par;

        iss >> cond >> par;
        assert(Level::failMeaningsMap.find(cond) != Level::failMeaningsMap.end());
        _currLevelProperies.fail = Level::failMeaningsMap.at(cond);
        _currLevelProperies.failParam = par;
    }

    // line 3 num of briefing lines
    {
        std::getline(f, line);
        int num;
        std::istringstream iss(line);
        iss >> num;
        assert(num>=0 && num < 10);
        // read briefing line by line
        for (int i = 0; i < num; i++) {
            std::getline(f, line);
            _currLevelProperies.briefing.push_back(line);
        }
    }

    // map size
    {
        std::string dummy;
        std::getline(f, line);
        std::istringstream iss(line);
        iss >> dummy >> map_w >> map_h;
        assert(dummy == "map");
        Logger::instance() << "Read map size: " << map_w << " " << map_h << "\n";
    }

    assert(map_w > 0 && map_w < globalConst::maxFieldWidth);
    assert(map_h > 0 && map_h < globalConst::maxFieldHeight);

    mapString.resize(map_w * map_h);

    // read segments
    {
        std::string type, size;
        std::getline(f, line);
        std::istringstream iss(line);
        iss >> type >> size;

        assert(type == "segments");
        assert(size == "7x7");
        int delimiterPos = size.find("x");
        segments_w = std::stoi(size.substr(0, delimiterPos));
        segments_h = std::stoi(size.substr(delimiterPos+1, size.length()));
        size_string = size;
    }

    // read the map
    {
        int segments_num_w = map_w / segments_w;
        int segments_num_h = map_h / segments_h;

        std::string segmentsFolderPath = "assets/maps/segments/" + size_string;
        for (int i = 0; i < segments_num_h; i++) {
            std::getline(f, line); // read serments line
            std::istringstream iss(line);
            // read row of segments
            int seg_count_x = 0;
            while(!iss.eof()) {
                std::string segment_name;
                iss >> segment_name;
                std::string segment_path = segmentsFolderPath + "/" + segment_name;
                namespace fs = std::filesystem;

                const fs::path dir(segment_path);
                const char *segp = segment_path.c_str();
                assert(fs::exists(dir) == true);
                assert(fs::is_directory(dir) == true);
                std::vector<std::string> segmentVariants;
                for (const auto& entry : fs::directory_iterator(dir)) {
                    if (!fs::is_regular_file(entry.status()))
                        continue; // ignore directories
                    std::string segFileName = entry.path().root_path().string() + entry.path().relative_path().string();
                    const char *segp = segFileName.c_str();
                    segmentVariants.push_back(segFileName);
                }

                assert(segmentVariants.size() > 0);

                // select random segment
                std::uniform_int_distribution<int> type_distr(0, segmentVariants.size()-1);
                int segmentIndex = type_distr(Utils::generator);
                assert(segmentIndex < segmentVariants.size());

                // read segment

                {
                    std::string segmentline;
                    const char *segp = segmentVariants[segmentIndex].c_str();
                    std::ifstream fsegment(segmentVariants[segmentIndex]);
                    std::getline(fsegment, segmentline);
                    std::string dummy;
                    int seg_w, seg_h;
                    std::istringstream isss(segmentline);
                    isss >> dummy >> seg_w >> seg_h;
                    assert(dummy == "segment");
                    assert(seg_w>0 && seg_w<16);
                    assert(seg_h>0 && seg_h<16);
                    assert(seg_w == segments_w);
                    assert(seg_h == segments_h);
                    int seg_row = 0;
                    while (std::getline(fsegment, segmentline)) {
                        int map_row = i*seg_h + seg_row;
                        std::istringstream issss(segmentline);
                        std::string seg;
                        issss >> seg;

                        for (int j=0; j<seg.length(); j++)
                            mapString[map_row*map_w + seg_count_x*seg_w + j] = seg[j];

                        seg_row++;

                    }
                }
                seg_count_x++;
            } // row of segments
        }
    }

    // read data for placing tank spawners
    while (std::getline(f, line)) {
        if (line.empty() || line[0] == '#')
            continue;
        std::istringstream iss(line);
        SpawnerData sd;
        iss >> sd.type >> sd.col >> sd.row >> sd.delay >> sd.timeout >> sd.quantity;

        assert(sd.type.empty() == false);
        assert(sd.row >= 0 && sd.row < globalConst::maxFieldWidth);
        assert(sd.col >= 0 && sd.col < globalConst::maxFieldHeight);
        assert(sd.delay >= 0 && sd.delay < 1000);
        assert(sd.timeout >= 0 && sd.timeout < 120);
        assert(sd.quantity > 0 && sd.quantity < 100);

        _spawners.push_back(sd);
    }

    return 0;
}

