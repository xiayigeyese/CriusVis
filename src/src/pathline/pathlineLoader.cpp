#include <map>

#include <pugixml/pugixml.hpp>

#include <agz/utility/string.h>

#include <crius/pathline/pathlineLoader.h>
#include <crius/utility/base64ToArray.h>

namespace
{

    struct PathlineIndices
    {
        int id   = -1;
        int time = -1;
        int x    = -1;
        int y    = -1;
        int z    = -1;
    };

    PathlineIndices findPathlineIndices(const pugi::xml_document &doc)
    {
        PathlineIndices result;

        auto items = doc.child("ParticleTracks").child("Items");
        for(auto item : items)
        {
            if(item.attribute("name").as_string() ==
                std::string_view("Particle X Position"))
                result.x = item.attribute("id").as_int();
            else if(item.attribute("name").as_string() ==
                std::string_view("Particle Y Position"))
                result.y = item.attribute("id").as_int();
            else if(item.attribute("name").as_string() ==
                std::string_view("Particle Z Position"))
                result.z = item.attribute("id").as_int();
            else if(item.attribute("name").as_string() ==
                std::string_view("Particle Time"))
                result.time = item.attribute("id").as_int();
            else if(item.attribute("name").as_string() ==
                std::string_view("Particle ID"))
                result.id = item.attribute("id").as_int();
        }

        if(result.x < 0 || result.y < 0 || result.z < 0 ||
           result.time < 0 || result.id < 0)
        {
            throw std::runtime_error(
                "failed to find pathline indices in xml file");
        }

        return result;
    }

    struct PathlineXMLData
    {
        pugi::xml_node id;
        pugi::xml_node time;
        pugi::xml_node x;
        pugi::xml_node y;
        pugi::xml_node z;
    };

    pugi::xml_node findPathlineComponentData(pugi::xml_node section, int index)
    {
        for(auto &data : section)
        {
            if(data.attribute("item").as_int() == index)
                return data;
        }

        throw std::runtime_error(
            "failed to find data with index " + std::to_string(index) +
            " in xml section");
    }

    PathlineXMLData findPathlineXMLData(
        pugi::xml_node section, const PathlineIndices &indices)
    {
        PathlineXMLData ret;
        ret.id   = findPathlineComponentData(section, indices.id);
        ret.time = findPathlineComponentData(section, indices.time);
        ret.x    = findPathlineComponentData(section, indices.x);
        ret.y    = findPathlineComponentData(section, indices.y);
        ret.z    = findPathlineComponentData(section, indices.z);
        return ret;
    }

    std::vector<int32_t> parseInt32Array(const char *str)
    {
        std::vector<int32_t> ret;
        auto int32Strs = agz::stdstr::split(agz::stdstr::trim(str), ' ', true);
        for(auto &s : int32Strs)
            ret.push_back(agz::stdstr::from_string<int32_t>(s));
        return ret;
    }

} // namespace anonymous

void PathlineLoader::loadFromXML(const std::string &filename)
{
    allPathlines_.clear();

    pugi::xml_document doc;
    if(!doc.load_file(filename.c_str()))
        throw std::runtime_error("failed to load/parse xml file: " + filename);

    const auto indices = findPathlineIndices(doc);

    std::map<int, Pathline> pathlines;

    for(auto section : doc.child("ParticleTracks").child("Tracks"))
    {
        if(section.name() != std::string_view("Section"))
            continue;

        auto xmlData  = findPathlineXMLData(section, indices);
        auto idData   = parseInt32Array(xmlData.id.text().get());
        auto timeData = base64ToFloatArray(xmlData.time.text().get());
        auto xData    = base64ToFloatArray(xmlData.x.text().get());
        auto yData    = base64ToFloatArray(xmlData.y.text().get());
        auto zData    = base64ToFloatArray(xmlData.z.text().get());

        if(idData.size() != xData.size() ||
           idData.size() != yData.size() ||
           idData.size() != zData.size() ||
           idData.size() != timeData.size())
            throw std::runtime_error("x/y/z/id/time data sizes are unmatched");

        for(size_t i = 0; i < idData.size(); ++i)
        {
            pathlines[idData[i]].points.push_back({
                timeData[i], { xData[i], yData[i], zData[i] }
            });
        }
    }

    Vec3 L(std::numeric_limits<float>::max());
    Vec3 H(std::numeric_limits<float>::lowest());
    for(auto &p : pathlines)
    {
        for(auto &tp : p.second.points)
        {
            L = elem_min(L, tp.position);
            H = elem_max(H, tp.position);
        }
    }

    const Vec3 offset = -0.5f * (L + H);
    const float scale = 1 / (std::max)((H - L).max_elem(), 0.001f);

    size_t maxLen = 0;

    for(auto &p : pathlines)
    {
        for(auto &tp : p.second.points)
            tp.position = scale * (tp.position + offset);

        std::sort(
            p.second.points.begin(), p.second.points.end(),
            [](const Timepoint &L, const Timepoint &R)
        {
            return L.time < R.time;
        });

        maxLen = std::max(maxLen, p.second.points.size());
        allPathlines_.push_back({ std::move(p.second.points) });
    }

    for(auto &p : allPathlines_)
    {
        if(p.points.size() < maxLen)
        {
            const auto newVal = p.points.back();
            p.points.resize(maxLen, newVal);
        }
    }
}

const std::vector<PathlineLoader::Pathline> &PathlineLoader::getAllPathlines() const noexcept
{
    return allPathlines_;
}
