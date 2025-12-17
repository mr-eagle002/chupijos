#include <iostream>
#include <vector>
#include <map>
#include <fstream>
#include <string>
#include <sstream> 
#include <algorithm>

using namespace std;
vector<string> split(const string &s, char delimiter) {
    vector<string> tokens;
    string token;
    istringstream tokenStream(s);
    while (getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

// =====================================================
// PERSONNE
// =====================================================
class Personne {
protected:
    string nom;
    string matricule;

public:
    Personne(string n, string m) : nom(n), matricule(m) {}
    virtual ~Personne() {}

    string getNom() const { return nom; }
    string getMatricule() const { return matricule; }

    bool estEtudiant() const { return matricule.size() >= 2 && matricule.substr(0, 2) == "ST"; }
    bool estProfesseur() const { return matricule.size() >= 2 && matricule.substr(0, 2) == "TE"; }

    virtual void afficher() const = 0;
};

// =====================================================
// ETUDIANT
// =====================================================
class Etudiant : public Personne {
private:
    map<string, vector<float> > notes;

public:
    Etudiant(string n, string m) : Personne(n, m) {}

    void ajouterNote(string matiere, float note) {
        notes[matiere].push_back(note);
    }

    float moyenneMatiere(string matiere) const {
        if (notes.find(matiere) == notes.end()) return 0;
        const vector<float>& v = notes.find(matiere)->second;
        float sum = 0;
        for (size_t i = 0; i < v.size(); i++) sum += v[i];
        return v.size() == 0 ? 0 : sum / v.size();
    }

    float moyenneGlobale() const {
        float total = 0;
        int count = 0;
        for (map<string, vector<float> >::const_iterator it = notes.begin(); it != notes.end(); ++it) {
            const vector<float>& v = it->second;
            for (size_t j = 0; j < v.size(); j++) {
                total += v[j];
                count++;
            }
        }
        return count == 0 ? 0 : total / count;
    }

    const map<string, vector<float> >& getNotes() const { return notes; }

    void afficher() const {
        cout << "Étudiant : " << nom << " (" << matricule << ")\n";
    }
};

// =====================================================
// PROFESSEUR
// =====================================================
class Professeur : public Personne {
private:
    vector<string> matieres;

public:
    Professeur(string n, string m) : Personne(n, m) {}

    void ajouterMatiere(string m) { matieres.push_back(m); }
    const vector<string>& getMatieres() const { return matieres; }

    void afficher() const {
        cout << "Professeur : " << nom << " (" << matricule << ")\n";
    }
};

// =====================================================
// ETABLISSEMENT
// =====================================================
class Etablissement {
private:
    vector<Personne*> personnes;

    void afficherDetails( Personne* p) const {
        p->afficher();

        Etudiant* e = dynamic_cast<Etudiant*>(p);
        if (e != NULL) {
            const map<string, vector<float> >& notes = e->getNotes();
            if (!notes.empty()) {
                cout << "  Notes:\n";
                for (const auto& pair : notes) {
                    cout << "    - " << pair.first << " : ";
                    for (float note : pair.second) cout << note << " ";
                    cout << " (Moy: " << e->moyenneMatiere(pair.first) << ")\n";
                }
            }
            cout << "  Moyenne globale : " << e->moyenneGlobale() << "\n";
        }

        Professeur* pr = dynamic_cast<Professeur*>(p);
        if (pr != NULL) {
            cout << "  Matières : ";
            for (const string& m : pr->getMatieres()) cout << m << ", ";
            cout << "\n";
        }
        cout << "---------------------------------\n";
    }

public:
    ~Etablissement() {
        for (size_t i = 0; i < personnes.size(); i++) delete personnes[i];
    }

    void ajouterPersonne(Personne* p) {
        personnes.push_back(p);
    }

    void chargerDonnees(const string& fichierCSV) {
        ifstream file(fichierCSV);
        if (!file.is_open()) {
            cerr << "Erreur: Impossible d'ouvrir le fichier " << fichierCSV << "\n";
            return;
        }

        string line;
        getline(file, line);

        while (getline(file, line)) {
            vector<string> fields = split(line, ',');
            if (fields.size() < 4) continue;

            string matricule = fields[0];
            string nom = fields[1];
            char type = fields[2][0];
            string details = fields[3];

            if (type == 'P') {
                Professeur* prof = new Professeur(nom, matricule);
                vector<string> matieres = split(details, ';');
                for (const string& m : matieres) {
                    if (!m.empty()) prof->ajouterMatiere(m);
                }
                ajouterPersonne(prof);
            }
            else if (type == 'E') {
                Etudiant* etu = new Etudiant(nom, matricule);
                vector<string> sujets_notes = split(details, ';');

                for (const string& sn : sujets_notes) {
                    if (sn.empty()) continue;
                    
                    vector<string> subject_and_notes = split(sn, ':');
                    if (subject_and_notes.size() != 2) continue;

                    string matiere = subject_and_notes[0];
                    string notes_str = subject_and_notes[1];

                    vector<string> note_values = split(notes_str, '|');
                    for (const string& nv : note_values) {
                        if (!nv.empty()) {
                            try {
                                etu->ajouterNote(matiere, stof(nv));
                            } catch (...) {}
                        }
                    }
                }
                ajouterPersonne(etu);
            }
        }
        cout << ">> Données chargées avec succès depuis " << fichierCSV << ".\n";
    }

    void afficherToutes() const {
        cout << "\n===== LISTE DE TOUTES LES PERSONNES (" << personnes.size() << " entrées) =====\n\n";
        for (const auto& p : personnes) {
            afficherDetails(p);
        }
    }

    void afficherSelection() const {
        string input;
        cout << "\nQue voulez-vous afficher?\n";
        cout << "  [P] : Professeurs\n";
        cout << "  [E] : Étudiants\n";
        cout << "  [T] : Tous (Toutes les personnes)\n";
        cout << "  [Q] : Quitter\n";
        cout << "Entrez votre choix : ";
        getline(cin, input);

        if (input.empty()) return;

        transform(input.begin(), input.end(), input.begin(), ::toupper);
        char choice = input[0];

        if (choice == 'Q') return;

        cout << "\n===== AFFICHAGE FILTRÉ (" << (choice == 'P' ? "Professeurs" : (choice == 'E' ? "Étudiants" : "Toutes")) << ") =====\n\n";
        int count = 0;

        for (const auto& p : personnes) {
            bool is_student = p->estEtudiant();
            bool is_teacher = p->estProfesseur();

            if (choice == 'T' || 
                (choice == 'E' && is_student) || 
                (choice == 'P' && is_teacher)) 
            {
                afficherDetails(p);
                count++;
            }
        }
        if (count == 0) {
            cout << "Aucune personne trouvée pour cette sélection.\n";
            cout << "---------------------------------\n";
        }
    }
};

// =====================================================
// MAIN
// =====================================================
int main() {
    Etablissement e;
    
    e.chargerDonnees("school_data.csv"); 

    string choice;
    do {
        e.afficherSelection();
        cout << "Appuyez sur Entrée pour continuer ou tapez 'Q' pour quitter. : ";
        getline(cin, choice);
        transform(choice.begin(), choice.end(), choice.begin(), ::toupper);

        if (choice == "Q") break;
    } while (true);

    cout << "\nProgramme terminé. Au revoir.\n";
    return 0;

}
