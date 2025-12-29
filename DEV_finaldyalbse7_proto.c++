#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <cstdlib>

using namespace std;

/* ===================== HELPERS ===================== */

// Simple number check (kept this as it doesn't manipulate strings)
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
        cout << "\n===== STUDENT HOME =====\n";
        cout << "Name      : " << nom << endl;
        cout << "Matricule : " << matricule << endl;
        for (map<string, vector<float> >::const_iterator it = notes.begin();
             it != notes.end(); ++it) {
            cout << "  " << it->first << " : ";
            for (size_t i = 0; i < it->second.size(); i++)
                cout << it->second[i] << " ";
            cout << endl;
        }
        cout << "Global average: " << moyenneGlobale() << endl;
    }

    void afficherAdmin() const {
        cout << nom << " | " << matricule << " | Student\n";
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

    bool enseigne(string mat) const {
        for (size_t i = 0; i < matieres.size(); i++)
            if (matieres[i] == mat) return true;
        return false;
    }

    void afficher() const {
        cout << "\n===== PROFESSOR HOME =====\n";
        cout << "Name      : " << nom << endl;
        cout << "Matricule : " << matricule << endl;
        cout << "Subjects  : ";
        for (size_t i = 0; i < matieres.size(); i++)
            cout << matieres[i] << " ";
        cout << endl;
    }

    void afficherAdmin() const {
        cout << nom << " | " << matricule << " | Teacher | ";
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
        cout << "\n===== ADMIN HOME =====\n";
        cout << "Administrator: " << nom << endl;
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
        // Debug print to see what we are comparing
        // cout << "[DEBUG] Searching for: '" << m << "'" << endl;
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
                cout << "User deleted.\n";
                return;
            }
        }
        cout << "User not found.\n";
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
        cout << "\n--- ALL USERS ---\n";
        for (size_t i = 0; i < users.size(); i++)
            users[i]->afficherAdmin();
    }
};

/* ===================== DATABASE LOADING ===================== */

void loadDatabase(Etablissement& e, const string& filename) {
    ifstream file(filename.c_str());
    
    // DEBUG CHECK 1: FILE OPENING
    if (!file.is_open()) {
        cerr << "\n[!] ERROR: Could not open file '" << filename << "'" << endl;
        cerr << "[!] Make sure 'school_db.csv' is in the same folder as the .exe file." << endl;
        return;
    }

    cout << "Loading database... " << endl;
    
    string line;
    // Skip Header
    if(!getline(file, line)) return; 

    int count = 0;

    while (getline(file, line)) {
        if (line.empty()) continue;

        // --- NEW PARSING LOGIC (No split/trim) ---
        stringstream ss(line);
        string segment;
        vector<string> row;

        // Split by comma
        while(getline(ss, segment, ',')) {
            // Optional: Manually remove \r if on Windows but reading in Linux style
            if (!segment.empty() && segment[segment.size()-1] == '\r') 
                segment.erase(segment.size()-1);
            row.push_back(segment);
        }

        if (row.size() < 4) continue;

        string matricule = row[0];
        string nom = row[1];
        string type = row[2];
        string details = row[3];

        // DEBUG CHECK 2: PRINT LOADED USER
        // This will verify if the file is actually being read
        cout << "[DEBUG] Loaded: " << matricule << " (" << nom << ")" << endl;

        if (type == "P") {
            Professeur* p = new Professeur(nom, matricule);
            
            // Parse details by semicolon
            stringstream dss(details);
            string subject;
            while(getline(dss, subject, ';')) {
                 if (!subject.empty() && subject[subject.size()-1] == '\r') 
                    subject.erase(subject.size()-1);
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
                 if (!part.empty() && part[part.size()-1] == '\r') 
                    part.erase(part.size()-1);
                 detailParts.push_back(part);
            }

            vector<string> subjects;
            vector<float> grades;

            for (size_t i = 0; i < detailParts.size(); ++i) {
                if (isNumber(detailParts[i])) {
                    grades.push_back(atof(detailParts[i].c_str()));
                } else {
                    if(!detailParts[i].empty()) subjects.push_back(detailParts[i]);
                }
            }

            // Grade Mapping
            if (subjects.size() == 1) {
                for (size_t i = 0; i < grades.size(); ++i)
                    st->ajouterNote(subjects[0], grades[i]);
            } else if (subjects.size() == grades.size()) {
                for (size_t i = 0; i < subjects.size(); ++i)
                    st->ajouterNote(subjects[i], grades[i]);
            } else {
                size_t limit = (subjects.size() < grades.size()) ? subjects.size() : grades.size();
                for (size_t i=0; i<limit; ++i)
                     st->ajouterNote(subjects[i], grades[i]);
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
    cout << "Finished loading " << count << " users.\n" << endl;
}

/* ===================== MAIN ===================== */
int main() {
    Etablissement e;

    e.ajouterUser(new Admin("SystemAdmin", "AD0001"));
    loadDatabase(e, "school_db.csv");

    while (true) {
        string m;
        cout << "\nLOGIN (matricule) or Q to quit: ";
        cin >> m;
        if (m == "Q") break;

        Personne* u = e.chercherParMatricule(m);
        if (!u) {
            cout << "❌ User not found.\n";
            continue;
        }

        // ... (Rest of logic remains same) ...
        if (u->getRole() == "ST") {
            u->afficher();
        }
        else if (u->getRole() == "TE") {
            Professeur* p = dynamic_cast<Professeur*>(u);
            p->afficher();
            cout << "\n--- MY STUDENTS ---\n";
            vector<Etudiant*> etudiants = e.getEtudiants();
            bool foundAny = false;

            for (size_t i = 0; i < etudiants.size(); i++) {
                map<string, vector<float> > notes = etudiants[i]->getNotes();
                bool hasSubject = false;
                for (map<string, vector<float> >::iterator it = notes.begin();
                     it != notes.end(); ++it) {
                    if (p->enseigne(it->first)) {
                        hasSubject = true;
                        break;
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
            if (!foundAny) cout << "(No students found for your subjects)\n";

            while (true) {
                cout << "\nA: Modify grade | Q: Logout\nChoice: ";
                string c; cin >> c;
                if (c == "Q") break;
                if (c == "A") {
                    string sm, mat; float note;
                    cout << "Student matricule: "; cin >> sm;
                    cout << "Subject: "; cin >> mat;
                    if (!p->enseigne(mat)) {
                        cout << "❌ Access denied.\n"; continue;
                    }
                    Etudiant* et = dynamic_cast<Etudiant*>(e.chercherParMatricule(sm));
                    bool takes = false;
                    if(et) {
                        map<string, vector<float> > n = et->getNotes();
                        if(n.find(mat)!=n.end()) takes=true;
                    }
                    if (et && takes) {
                        cout << "Grade: "; cin >> note;
                        et->modifierDerniereNote(mat, note);
                        cout << "✔ Grade updated.\n";
                    } else cout << "❌ Error.\n";
                }
            }
        }
        else if (u->getRole() == "AD") {
            u->afficher();
            e.afficherPourAdmin();
            while (true) {
                cout << "\nA: Add | D: Delete | Q: Logout\nChoice: ";
                string c; cin >> c;
                if (c == "Q") break;
                if (c == "A") {
                    string n, m2, r;
                    cout << "Name: "; cin >> n;
                    cout << "Matricule: "; cin >> m2;
                    cout << "Role (ST/TE): "; cin >> r;
                    if (r == "ST") e.ajouterUser(new Etudiant(n, m2));
                    else if (r == "TE") {
                        Professeur* p = new Professeur(n, m2);
                        string mat; cout << "Subject: "; cin >> mat;
                        p->ajouterMatiere(mat); e.ajouterUser(p);
                    }
                }
                if (c == "D") {
                    string dm; cout << "Matricule: "; cin >> dm;
                    e.supprimerUser(dm);
                }
                e.afficherPourAdmin();
            }
        }
    }
    return 0;
}