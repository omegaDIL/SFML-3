/*******************************************************************
 * \file   GUI.hpp
 * \brief  Group all entities for creating and managing graphical user interfaces.
 *
 * \author OmegaDIL.
 * \date   July 2024.
 *
 * \note This file depends on the SFML library.
 *********************************************************************/

#ifndef GUI_HPP
#define GUI_HPP

#include "GUI/GraphicalResources.hpp"
#include "GUI/BasicInterface.hpp"
#include "GUI/MutableInterface.hpp"
#include "GUI/InteractiveInterface.hpp"
#include "GUI/AdvancedInterface.hpp"

using BGUI = gui::BasicInterface;
using MGUI = gui::MutableInterface;
using IGUI = gui::InteractiveInterface; // Often enough for most apps
using AGUI = gui::AdvancedInterface;


#include <string>
#include <sstream>

/**
 * @brief Creates a new instance of a window to display an error message.
 * @complexity constant O(1).
 *
 * @param[in] errorTitle: The title of the window.
 * @param[in] errorMessage: The message to be displayed.
 *
 * @note This function is blocking and will terminate once the user closes the new window.
 * @note Don't forget to put the character \n to avoid the text to not be seen entirely when you
 *       have a long line.
 *
 * @see `sf::RenderWindow`, `IGUI`.
 */
void showErrorsUsingWindow(const std::string& errorTitle, const std::ostringstream& errorMessage) noexcept;

/**
 * \brief Initializes the interface
 * 
 * \param[out] gui: the graphical user inteface to initialize.
 * 
 * \warning Assert if gui is nulltpr
 */
void populateGUI(IGUI* gui) noexcept; 

// TODO: complete the function `populateGUI` your own way.
// You can change the gui type, add arguments, or add more
// interfaces to populate. Feel free.

// TODO: id -> IDENTIFIER
// TODO: find().second -> at() + faire des reserves pour les maps et les vectors afin d'eviter les reallocations de memoire.
// TODO: _ + mqb + to_string -> _ + to_string + _ + mqb
// TODO: mettre __ au lieu de _ lorsque l'utilisateur ne doit pas interagir avec l'élément (ex: __cursorEditing pour le curseur d'édition de texte)
// TODO: ajouter aux lambdas un argument pour l'interface courante.
// TODO: mettre des int a la place des unsigned int pour les MQB, car c le bordel de faire -1 et on aura jamais plus de 2 millards de boxes dans 1 mqb
// TODO: see attribute [[assume]] for save and its wrappers
// TODO: rendre coherent le nommage des elements dynamiques -> __ + id quand ajoute par Interface && __nom__ quand ajoute par interface + sans id
// TODO: permettre de decocher tous les mqb.
// TODO: remove pointer from interactable element
// TODO: module C++20
// TODO: Ajouter une std::function pour les mqb
// TODO: string_view
//TODO: changer hash

#endif // GUI_HPP