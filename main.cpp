/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.cpp
 * Author: josepva
 *
 * Created on 24 de março de 2020, 08:16
 */

#include <cstdlib>
#define LIBEMBER_HEADER_ONLY
#include <ember/Ember.hpp>
#include <ember/glow/GlowNodeFactory.hpp>
#include <ember/glow/GlowNode.hpp>
#include <ember/glow/GlowRootElementCollection.hpp>
#include <ember/dom/AsyncBerReader.hpp>
#include <ember/dom/DomReader.hpp>
#include <s101/StreamEncoder.hpp>
#include <s101/StreamDecoder.hpp>
#include <ember/util/OctetStream.hpp>
#include <iostream>

using namespace std;

/**
 * Imprime o conteúdo de um libember::util::OctetStream
 * @param O libember::util::OctetStream a ser impresso
 */
void list_stream(libember::util::OctetStream& stream){
    std::cout << "Stream size: " << std::dec << stream.size() << std::endl;
    int counter = 0;
    std::string ascii = "";
    for(auto it = stream.begin(); it!=stream.end();)
    {
        if((counter%16)==0)
            std::cout << std::hex << std::setw(4) << std::setfill('0') << counter << " - ";
        ascii += (*it>=32 && *it<=126 ? *it : '.');
        std::cout << std::hex<< std::setw(2) << int(*it) << " ";
        it++;
        if((counter%16)==15 || it==stream.end())
        {
            std::cout << ascii << std::endl;
            ascii = "";
        }
        counter++;
    }
}

/**
 * Ponto de entrada do programa
 */
int main(int argc, char** argv) {
    // Toda a estrutura de dados começa com um libember::glow::GlowRootElementCollection
    libember::glow::GlowRootElementCollection* root = new libember::glow::GlowRootElementCollection;
    
    libember::glow::GlowNode* switches = new libember::glow::GlowNode(root, 1);
    switches->setIdentifier("Switches");
    libember::glow::GlowNode* switch1 = new libember::glow::GlowNode(switches, 1);
    switch1->setIdentifier("Switch1");
    libember::glow::GlowNode* switch2 = new libember::glow::GlowNode(switches, 2);
    switch2->setIdentifier("Switch2");
    libember::glow::GlowNode* switch3 = new libember::glow::GlowNode(switches, 3);
    switch3->setIdentifier("Switch3");
    libember::glow::GlowParameter* on_off1 = new libember::glow::GlowParameter(switch1, 1);
    on_off1->setIdentifier("On_Off");
    on_off1->setDescription("Turn switch on or off");
    on_off1->setAccess(libember::glow::Access::ReadWrite);
    on_off1->setType(libember::glow::ParameterType::Boolean);
    on_off1->setValue(false);
    libember::glow::GlowParameter* on_off2 = new libember::glow::GlowParameter(switch2, 1);
    on_off2->setIdentifier("On_Off");
    on_off2->setDescription("Turn switch on or off");
    on_off2->setAccess(libember::glow::Access::ReadWrite);
    on_off2->setType(libember::glow::ParameterType::Boolean);
    on_off2->setValue(true);
    libember::glow::GlowParameter* on_off3 = new libember::glow::GlowParameter(switch3, 1);
    on_off3->setIdentifier("On_Off");
    on_off3->setDescription("Turn switch on or off");
    on_off3->setAccess(libember::glow::Access::ReadWrite);
    on_off3->setType(libember::glow::ParameterType::Boolean);
    on_off3->setValue(false);
    
    libember::util::OctetStream stream;
    root->encode(stream);
    
    list_stream(stream);

    libs101::StreamEncoder<> encoder;
    encoder.encode(stream.begin(),stream.end());
    
    // Você precisa indicar para o encoder que não quer adicionar mais dados.
    // Com isso ele adiciona no final do stream o CRC16 e o valor 0xFF para
    // indicar fim da mensagem
    encoder.finish();
    
    libember::util::OctetStream encoded_stream;
    // Para finalizar o stream encodado é transferido para um OctetStream aonde
    // pode ser finalmente transmitido para o dispositivo ou software de controle.
    encoded_stream.append(encoder.begin(),encoder.end());
    
    list_stream(encoded_stream);
    
    // Iniciado agora o processo inverso de decodificação

    libember::util::OctetStream decoded_stream;
    libs101::StreamDecoder<> decoder;

    // 
    decoder.read(encoded_stream.begin(),
                 encoded_stream.end(),
                 [&](auto it,auto end)
                 {
                     decoded_stream.append(it,end);
                     std::cout << "Chamou o callback." << std::endl;
                 });

    list_stream(decoded_stream);
    libember::dom::DomReader reader;
    libember::dom::Node* decoded_tree = reader.decodeTree(decoded_stream,libember::glow::GlowNodeFactory::getFactory());
    libember::glow::GlowRootElementCollection* decoded_tree_glow = dynamic_cast<libember::glow::GlowRootElementCollection*>(decoded_tree);
    decoded_tree_glow->update();
    
    auto it = decoded_tree_glow->begin();
    auto end = decoded_tree_glow->end();
    for(;it!=end;it++)
    {
        libember::dom::Node& node = *(it);
        libember::glow::GlowNode& node_glow = dynamic_cast<libember::glow::GlowNode&>(node);
        std::cout << "Identifier: " << node_glow.identifier() << std::endl;
        libember::glow::GlowElementCollection* node_glow_children = node_glow.children();
        auto it_child = node_glow_children->begin();
        auto end_child = node_glow_children->end();
        for(;it_child!=end_child;it_child++)
        {
            libember::dom::Node& node = *(it_child);
            libember::glow::GlowNode& node_glow = dynamic_cast<libember::glow::GlowNode&>(node);
            std::cout << "\tIdentifier: " << node_glow.identifier() << std::endl;
            libember::glow::GlowElementCollection* node_glow_children = node_glow.children();
            auto it_parm = node_glow_children->begin();
            libember::dom::Node& node_parm = *(it_parm);
            libember::glow::GlowParameter& node_parm_glow = dynamic_cast<libember::glow::GlowParameter&>(node_parm);
            std::cout << "\t\tIdentifier: " << node_parm_glow.identifier() << std::endl;
            std::cout << "\t\tDescription: " << node_parm_glow.description() << std::endl;
            std::cout << "\t\tValue: " << node_parm_glow.value().toBoolean() << std::endl;
        }
    }   
    delete root;
    
    return 0;
}

