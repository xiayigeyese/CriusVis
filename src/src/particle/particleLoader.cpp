#include <pugixml/pugixml.hpp>

#include <crius/particle/particleLoader.h>
#include <crius/utility/base64ToArray.h>

namespace
{
    struct ParticleInfoIndices
    {
        int x = -1;
        int y = -1;
        int z = -1;
        int colorBy = -1;
    };

    ParticleInfoIndices findParticlePositionIndices(const pugi::xml_document &doc)
    {
        ParticleInfoIndices result;

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
                    std::string_view("Particle X Velocity") && result.colorBy < 0)
                result.colorBy = item.attribute("id").as_int();
            else if(item.attribute("name").as_string() == 
                    std::string_view("COLORBY"))
                result.colorBy = item.attribute("id").as_int();
        }

        if(result.x < 0 || result.y < 0 || result.z < 0 || result.colorBy < 0)
        {
            throw std::runtime_error(
                "failed to find particle info indices in xml file");
        }

        return result;
    }

    struct ParticleXMLData
    {
        pugi::xml_node x;
        pugi::xml_node y;
        pugi::xml_node z;
        pugi::xml_node colorBy;
    };

    pugi::xml_node findParticleComponentData(pugi::xml_node section, int index)
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

    ParticleXMLData findPositionData(
        pugi::xml_node section, const ParticleInfoIndices &indices)
    {
        ParticleXMLData ret;
        ret.x       = findParticleComponentData(section, indices.x);
        ret.y       = findParticleComponentData(section, indices.y);
        ret.z       = findParticleComponentData(section, indices.z);
        ret.colorBy = findParticleComponentData(section, indices.colorBy);
        return ret;
    }
}

void ParticleLoader::loadFromXML(const std::string &filename)
{
    allParticles_.clear();

    pugi::xml_document doc;
    if(!doc.load_file(filename.c_str()))
        throw std::runtime_error("failed to load/parse xml file: " + filename);

    const auto indices = findParticlePositionIndices(doc);

    for(auto section : doc.child("ParticleTracks").child("Tracks"))
    {
        if(section.name() != std::string_view("Section"))
            continue;

        auto data = findPositionData(section, indices);
        auto xData = base64ToFloatArray(data.x.text().get());
        auto yData = base64ToFloatArray(data.y.text().get());
        auto zData = base64ToFloatArray(data.z.text().get());
        auto colorByData = base64ToFloatArray(data.colorBy.text().get());

        if(xData.size() != yData.size() ||
           xData.size() != zData.size() ||
           xData.size() != colorByData.size())
            throw std::runtime_error("x/y/z data sizes are unmatched");

        for(size_t i = 0; i < xData.size(); ++i)
        {
            allParticles_.push_back(
                { { xData[i], yData[i], zData[i] }, colorByData[i] });
        }
    }

    Vec3 L(std::numeric_limits<float>::max());
    Vec3 H(std::numeric_limits<float>::lowest());
    for(auto &p : allParticles_)
    {
        L = elem_min(L, p.position);
        H = elem_max(H, p.position);
    }

    const Vec3 offset = -0.5f * (L + H);
    const float scale = 1 / (std::max)((H - L).max_elem(), 0.001f);

    for(auto &p : allParticles_)
        p.position = scale * (p.position + offset);
}

const std::vector<ParticleLoader::Particle> &
ParticleLoader::getAllParticles() const noexcept
{
    return allParticles_;
}
