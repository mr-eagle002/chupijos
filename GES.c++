#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <cstdlib>

using namespace std;

/* ===================== FONCTIONS UTILES ===================== */

bool isNumber(const string& s) {
    if (s.empty()) return false;
    char* end = 0;
    strtod(s.c_str(), &end);
    return end != s.c_str() && *end == '\0';
}

/* ===================== CLASSES ===================== */

class Personne {
protected:
    string nom;
    string matricule;
    string role; 

public:
    Personne(string n, string m, string r)
        : nom(n), matricule(m), role(r) {}

    virtual ~Personne() {}

    string getNom() const { return nom; }
    string getMatricule() const { return matricule; }
    string getRole() const { return role; }

    virtual void afficher() const = 0;
    virtual void afficherAdmin() const = 0;
};

class Etudiant : public Personne {
private:
    map<string, vector<float> > notes;

public:
    Etudiant(string n, string m)
        : Personne(n, m, "ST") {}

    void ajouterNote(string mat, float note) {
        notes[mat].push_back(note);
    }

    void modifierDerniereNote(string mat, float note) {
        if (!notes[mat].empty())
            notes[mat].back() = note;
        else
            notes[mat].push_back(note);
    }

    map<string, vector<float> > getNotes() const {
        return notes;
    }

    float moyenneGlobale() const {
        float s = 0;
        int c = 0;
        for (map<string, vector<float> >::const_iterator it = notes.begin();
             it != notes.end(); ++it) {
            for (size_t i = 0; i < it->second.size(); i++) {
                s += it->second[i];
                c++;
            }
        }
        return c ? s / c : 0;
    }

    void afficher() const {
        cout << "\n===== ACCUEIL ÉTUDIANT =====\n";
        cout << "Nom             : " << nom << endl;
        cout << "Matricule       : " << matricule << endl;
        for (map<string, vector<float> >::const_iterator it = notes.begin();
             it != notes.end(); ++it) {
            cout << "  " << it->first << " : ";
            for (size_t i = 0; i < it->second.size(); i++)
                cout << it->second[i] << " ";
            cout << endl;
        }
        cout << "Moyenne globale : " << moyenneGlobale() << endl;
    }

    void afficherAdmin() const {
        cout << nom << " | " << matricule << " | Étudiant\n";
    }
};

class Professeur : public Personne {
private:
    vector<string> matieres;

public:
    Professeur(string n, string m)
        : Personne(n, m, "TE") {}

    void ajouterMatiere(string mat) {
        matieres.push_back(mat);
    }

    // NOUVEAU : Getter nécessaire pour la sauvegarde
    vector<string> getMatieres() const {
        return matieres;
    }

    bool enseigne(string mat) const {
        for (size_t i = 0; i < matieres.size(); i++)
            if (matieres[i] == mat) return true;
        return false;
    }

    void afficher() const {
        cout << "\n===== ACCUEIL PROFESSEUR =====\n";
        cout << "Nom       : " << nom << endl;
        cout << "Matricule : " << matricule << endl;
        cout << "Matières  : ";
        for (size_t i = 0; i < matieres.size(); i++)
            cout << matieres[i] << " ";
        cout << endl;
    }

    void afficherAdmin() const {
        cout << nom << " | " << matricule << " | Professeur | ";
        for (size_t i = 0; i < matieres.size(); i++)
            cout << matieres[i] << " ";
        cout << endl;
    }
};

class Admin : public Personne {
public:
    Admin(string n, string m)
        : Personne(n, m, "AD") {}

    void afficher() const {
        cout << "\n===== ACCUEIL ADMIN =====\n";
        cout << "Administrateur : " << nom << endl;
    }

    void afficherAdmin() const {
        cout << nom << " | " << matricule << " | Admin\n";
    }
};

class Etablissement {
private:
    vector<Personne*> users;

public:
    ~Etablissement() {
        for (size_t i = 0; i < users.size(); i++)
            delete users[i];
    }

    void ajouterUser(Personne* p) {
        users.push_back(p);
    }

    Personne* chercherParMatricule(string m) {
        for (size_t i = 0; i < users.size(); i++) {
            if (users[i]->getMatricule() == m)
                return users[i];
        }
        return NULL;
    }

    void supprimerUser(string m) {
        for (size_t i = 0; i < users.size(); i++) {
            if (users[i]->getMatricule() == m) {
                delete users[i];
                users.erase(users.begin() + i);
                cout << "Utilisateur supprimé.\n";
                return;
            }
        }
        cout << "Utilisateur introuvable.\n";
    }

    vector<Etudiant*> getEtudiants() const {
        vector<Etudiant*> res;
        for (size_t i = 0; i < users.size(); i++) {
            Etudiant* e = dynamic_cast<Etudiant*>(users[i]);
            if (e) res.push_back(e);
        }
        return res;
    }

    void afficherPourAdmin() const {
        cout << "\n--- TOUS LES UTILISATEURS ---\n";
        for (size_t i = 0; i < users.size(); i++)
            users[i]->afficherAdmin();
    }

    // NOUVEAU : Fonction de sauvegarde
    void sauvegarder(const string& filename) const {
        ofstream file(filename.c_str());
        if (!file.is_open()) {
            cerr << "Erreur : Impossible de sauvegarder le fichier." << endl;
            return;
        }

        // Écriture de l'en-tête
        file << "matricule,nom,type,details\n";

        for (size_t i = 0; i < users.size(); ++i) {
            Personne* p = users[i];
            file << p->getMatricule() << "," << p->getNom() << ",";

            if (p->getRole() == "AD") {
                file << "A,"; 
                // Admin n'a pas de détails
            }
            else if (p->getRole() == "TE") {
                file << "P,";
                Professeur* pr = dynamic_cast<Professeur*>(p);
                if (pr) {
                    vector<string> mats = pr->getMatieres();
                    for (size_t j = 0; j < mats.size(); ++j) {
                        file << mats[j];
                        if (j < mats.size() - 1) file << ";";
                    }
                }
            }
            else if (p->getRole() == "ST") {
                file << "E,";
                Etudiant* et = dynamic_cast<Etudiant*>(p);
                if (et) {
                    // Reconstruction du format : Matiere;Note;Matiere;Note
                    map<string, vector<float> > n = et->getNotes();
                    bool first = true;
                    for (map<string, vector<float> >::iterator it = n.begin(); it != n.end(); ++it) {
                        for (size_t k = 0; k < it->second.size(); ++k) {
                            if (!first) file << ";";
                            file << it->first << ";" << it->second[k];
                            first = false;
                        }
                    }
                }
            }
            file << "\n";
        }
        file.close();
    }
};

/* ===================== CHARGEMENT BDD ===================== */

void loadDatabase(Etablissement& e, const string& filename) {
    ifstream file(filename.c_str());
    
    if (!file.is_open()) {
        cerr << "\n[!] ERREUR : Impossible d'ouvrir le fichier '" << filename << "'" << endl;
        return;
    }

    cout << "Chargement de la base de données... " << endl;
    
    string line;
    if(!getline(file, line)) return; // Skip header

    int count = 0;

    while (getline(file, line)) {
        if (line.empty()) continue;

        stringstream ss(line);
        string segment;
        vector<string> row;

        while(getline(ss, segment, ',')) {
            if (!segment.empty() && segment[segment.size()-1] == '\r') 
                segment.erase(segment.size()-1);
            row.push_back(segment);
        }

        if (row.size() < 4) continue;

        string matricule = row[0];
        string nom = row[1];
        string type = row[2];
        string details = row[3];

        if (type == "P") {
            Professeur* p = new Professeur(nom, matricule);
            stringstream dss(details);
            string subject;
            while(getline(dss, subject, ';')) {
                 if (!subject.empty() && subject[subject.size()-1] == '\r') subject.erase(subject.size()-1);
                 if(!subject.empty()) p->ajouterMatiere(subject);
            }
            e.ajouterUser(p);
            count++;
        }
        else if (type == "E") {
            Etudiant* st = new Etudiant(nom, matricule);
            vector<string> detailParts;
            stringstream dss(details);
            string part;
            while(getline(dss, part, ';')) {
                 if (!part.empty() && part[part.size()-1] == '\r') part.erase(part.size()-1);
                 detailParts.push_back(part);
            }
            vector<string> subjects;
            vector<float> grades;
            for (size_t i = 0; i < detailParts.size(); ++i) {
                if (isNumber(detailParts[i])) grades.push_back(atof(detailParts[i].c_str()));
                else if(!detailParts[i].empty()) subjects.push_back(detailParts[i]);
            }
            if (subjects.size() == 1) {
                for (size_t i = 0; i < grades.size(); ++i) st->ajouterNote(subjects[0], grades[i]);
            } else if (subjects.size() == grades.size()) {
                for (size_t i = 0; i < subjects.size(); ++i) st->ajouterNote(subjects[i], grades[i]);
            } else {
                size_t limit = (subjects.size() < grades.size()) ? subjects.size() : grades.size();
                for (size_t i=0; i<limit; ++i) st->ajouterNote(subjects[i], grades[i]);
            }
            e.ajouterUser(st);
            count++;
        }
        else if (type == "A") {
            e.ajouterUser(new Admin(nom, matricule));
            count++;
        }
    }
    file.close();
    cout << "Chargement terminé de " << count << " utilisateurs.\n" << endl;
}

/* ===================== MAIN ===================== */
int main() {
    Etablissement e;
    string dbName = "school_db.csv"; // Nom du fichier stocké dans une variable

    loadDatabase(e, dbName);

    while (true) {
        string m;
        cout << "\nCONNEXION (matricule) ou Q pour quitter : ";
        cin >> m;
        if (m == "Q" || m == "q") break;

        Personne* u = e.chercherParMatricule(m);
        if (!u) {
            cout << "Utilisateur introuvable.\n";
            continue;
        }

        if (u->getRole() == "ST") {
            u->afficher();
        }
        else if (u->getRole() == "TE") {
            Professeur* p = dynamic_cast<Professeur*>(u);
            p->afficher();
            cout << "\n--- MES ÉTUDIANTS ---\n";
            vector<Etudiant*> etudiants = e.getEtudiants();
            bool foundAny = false;

            for (size_t i = 0; i < etudiants.size(); i++) {
                map<string, vector<float> > notes = etudiants[i]->getNotes();
                bool hasSubject = false;
                for (map<string, vector<float> >::iterator it = notes.begin();
                     it != notes.end(); ++it) {
                    if (p->enseigne(it->first)) {
                        hasSubject = true; break;
                    }
                }
                if (hasSubject) {
                    foundAny = true;
                    cout << etudiants[i]->getNom() << " | " << etudiants[i]->getMatricule() << " | ";
                    for (map<string, vector<float> >::iterator it = notes.begin();
                         it != notes.end(); ++it) {
                        if (p->enseigne(it->first)) {
                            cout << it->first << ": ";
                            for (size_t j = 0; j < it->second.size(); j++)
                                cout << it->second[j] << " ";
                            cout << "| ";
                        }
                    }
                    cout << endl;
                }
            }
            if (!foundAny) cout << "(Aucun étudiant trouvé pour vos matières)\n";

            while (true) {
                cout << "\nA : Modifier note | Q : Déconnexion\nChoix : ";
                string c; cin >> c;
                if (c == "Q" || c == "q") break;
                if (c == "A"|| c == "a") {
                    string sm, mat; float note;
                    cout << "Matricule étudiant : "; cin >> sm;
                    cout << "Matière : "; cin >> mat;
                    if (!p->enseigne(mat)) {
                        cout << "Accès refusé.\n"; continue;
                    }
                    Etudiant* et = dynamic_cast<Etudiant*>(e.chercherParMatricule(sm));
                    bool takes = false;
                    if(et) {
                        map<string, vector<float> > n = et->getNotes();
                        if(n.find(mat)!=n.end()) takes=true;
                    }
                    if (et && takes) {
                        cout << "Note : "; cin >> note;
                        et->modifierDerniereNote(mat, note);
                        cout << "Note mise à jour.\n";

                        // SAUVEGARDE AUTOMATIQUE
                        e.sauvegarder(dbName);
                        cout << "Base de données sauvegardée.\n";

                    } else cout << "Erreur.\n";
                }
            }
        }
        else if (u->getRole() == "AD") {
            u->afficher();
            e.afficherPourAdmin();
            while (true) {
                cout << "\nA : Ajouter | D : Supprimer | Q : Déconnexion\nChoix : ";
                string c; cin >> c;
                if (c == "Q" || c == "q") break;
                if (c == "A"|| c == "a") {
                    string n, m2, r;
                    cout << "Nom : "; cin >> n;
                    cout << "Matricule : "; cin >> m2;
                    cout << "Rôle (ST/TE) : "; cin >> r;
                    if (r == "ST") e.ajouterUser(new Etudiant(n, m2));
                    else if (r == "TE") {
                        Professeur* p = new Professeur(n, m2);
                        string mat; cout << "Matière : "; cin >> mat;
                        p->ajouterMatiere(mat); e.ajouterUser(p);
                    }
                    // SAUVEGARDE AUTOMATIQUE
                    e.sauvegarder(dbName);
                }
                if (c == "D"|| c == "d") {
                    string dm; cout << "Matricule : "; cin >> dm;
                    e.supprimerUser(dm);
                    // SAUVEGARDE AUTOMATIQUE
                    e.sauvegarder(dbName);
                }
                e.afficherPourAdmin();
            }
        }
    }
    return 0;
}
