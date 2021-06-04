#include"linker.hpp"
#include"linker_exceptions.hpp"
#include<iomanip>
#include<fstream>
#include<sstream>
#include<algorithm>
#include<unordered_set>

linker * linker::instance = nullptr;

void linker::link_file(std::ifstream & file)
{
    std::string line;

    //SYMBOLS
    while(std::getline(file, line))
    {
        if(line == "")
            break;
        
        std::stringstream ss(line);
        linker::symbol sym;
        ss >> std::hex >> sym.name >> sym.val >> sym.section_abs;
        if(sym.section_abs == "")
            sym.section_abs = "UND";
        add_symbol(sym);
    }

    //RELOCATIONS
    while(std::getline(file, line))
    {
        if(line == "")
            break;
        
        std::stringstream ss(line);
        std::string section;
        short size;
        ss >> std::hex >> section >> size;
        while(size--)
        {
            std::getline(file, line);
            std::stringstream ss(line);
            linker::relocation rel;
            ss >> std::hex >> rel.location >> rel.pc_rel >> rel.big_endian >> rel.symbol;
            add_relocation(rel, section);
        }
    }

    //BYTES
    while(std::getline(file, line))
    {
        if(line == "")
            break;
        
        std::stringstream ss(line);
        std::string section;
        ss >> section;
        std::getline(file, line);
        std::stringstream file_bytes(line);
        unsigned short byte;
        while(file_bytes >> std::hex >> byte)
        {
            bytes[section].push_back(byte);
        }
    }
}

void linker::check_symbols()
{
    for(std::pair<const std::string, linker::symbol> & sym : symbols)
    {
        if(sym.second.section_abs == "UND")
        {
            throw linker_exception("Undefined symbol " + sym.first);
        }
    }
}

void linker::fix_relocations()
{
    for(std::pair<std::string, std::vector<relocation>> rels : relocations)
    {
        std::string section = rels.first;
        for(relocation rel : rels.second)
        {
            if(!get_symbol(rel.symbol).is_section())
            {
                add_word(get_symbol(rel.symbol).val, rel.big_endian, section, rel.location);
            }
            else
            {
                add_word(rel.addend, rel.big_endian, section, rel.location);
            }
            add_word(section_locations[get_symbol(rel.symbol).section_abs], rel.big_endian, section, rel.location);
            if(rel.pc_rel)
            {
                add_word(-rel.location, rel.big_endian, section, rel.location);
                add_word(-section_locations[section], rel.big_endian, section, rel.location);
            }
        }
    }
}

void linker::generate_linkable(std::ofstream & out)
{
    for(std::pair<std::string, linker::symbol> e : symbols)
    {
        
        out<<std::hex;
        out<<e.first<<" ";
        out<<e.second.val<<" ";
        out<<e.second.section_abs<<"\n";
    }
    out<<"\n";
    for(std::pair<const std::string, std::vector<relocation>> & rel_list : relocations)
    {
        out<<rel_list.first<<" "<<rel_list.second.size()<<"\n";
        for(relocation & rel : rel_list.second)
        {
            if(rel.pc_rel && get_symbol(rel.symbol).section_abs == rel_list.first)
            {
                add_word(-rel.location, rel.big_endian, rel_list.first, rel.location);
                if(get_symbol(rel.symbol).is_section())
                    add_word(rel.addend, rel.big_endian, rel_list.first, rel.location);
                add_word(get_symbol(rel.symbol).val, rel.big_endian, rel_list.first, rel.location);
            }
            else
            {
                out<<rel.location<<" "<<rel.pc_rel<<" "<<rel.big_endian<<" "<<rel.symbol<<"\n";
            }
        }
    }
    out<<"\n";
    for(std::pair<const std::string, std::vector<unsigned char>> & byte_list : bytes)
    {
        if(byte_list.first == "#default")
            continue;
        out<<byte_list.first<<"\n";
        for(unsigned char & b : byte_list.second)
        {
            out<<(unsigned int)b<<" ";
        }
        out<<"\n";
    }
}

void linker::generate_executable(std::ofstream & file)
{
    short loc = 0x8;
    unsigned short row = -8;
    file << std::hex << std::setfill('0');
    for(std::pair<unsigned short, std::string> & section : section_starts)
    {
        for(unsigned char & byte : bytes[section.second])
        {
            if(!(loc ^ 0x8))
            {
                loc = 0;
                row += 8;
                if(row)
                    file << "\n";
                file << std::setw(4) << row << ": ";
            }
            file << std::setw(2) << (unsigned short)byte << " ";
            ++ loc;
        }
    }
}

void linker::fix_sections()
{
    std::sort(section_starts.begin(), section_starts.end());
    std::unordered_set<std::string> all_sections;
    for(auto section : bytes)
    {
        all_sections.insert(section.first);
    }
    unsigned short prev = 0;
    std::string prev_section = "#default";
    section_locations[prev_section] = 0;
    all_sections.erase("#default");
    for(std::pair<unsigned short, std::string> & start : section_starts)
    {
        if(all_sections.find(start.second) == all_sections.end())
            throw linker_exception("Missing or duplicate section " + start.second);
        if(prev > start.first)
            throw linker_exception("Section overlap, sections " + start.second + " and " + prev_section);
        if(prev < start.first)
        {
            unsigned short no_bytes = start.first - prev;
            while(no_bytes--)
            {
                bytes[prev_section].push_back(0);
            }
        }
        section_locations[start.second] = start.first;
        prev = start.first + bytes[start.second].size();
        prev_section = start.second;
        all_sections.erase(prev_section);
    }
    for(const std::string & section : all_sections)
    {
        section_locations[section] = prev;
        prev += bytes[section].size();
    }
    section_starts.clear();
    for(std::pair<const std::string, unsigned short> & section : section_locations)
    {
        section_starts.push_back({section.second, section.first});
    }
    std::sort(section_starts.begin(), section_starts.end());
}

void linker::add_symbol(linker::symbol sym)
{
    if(symbol_defined(sym.name))
    {
        if((sym.section_abs == "UND") || (sym.is_section()))return;
        throw linker_exception("Duplicate symbol " + sym.name);
    }
    if(!sym.is_section())sym.val += get_offset(sym.section_abs);
    get_symbol(sym.name) = sym;
}

void linker::add_relocation(linker::relocation rel, std::string section)
{
    rel.location += get_offset(section);
    if(get_symbol(rel.symbol).is_section())
        rel.addend = get_offset(get_symbol(rel.symbol).section_abs);
    relocations[section].push_back(rel);
}

void linker::add_word(unsigned short add, bool big_endian, std::string section, unsigned short location)
{
    unsigned short prev = bytes[section][location];
    if(big_endian)
    {
        prev <<= 8;
        prev += bytes[section][location + 1];
    }
    else
    {
        prev += (unsigned short)(bytes[section][location + 1]) << 8;
    }
    prev += add;
    if(big_endian)
    {
        bytes[section][location + 1] = prev;
        bytes[section][location] = prev >> 8;
    }
    else
    {
        bytes[section][location + 1] = prev >> 8;
        bytes[section][location] = prev;
    }
}