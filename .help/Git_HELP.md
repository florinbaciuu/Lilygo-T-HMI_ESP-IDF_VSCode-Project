# 📘 Ghid Git personal – by florinbaciuu (a.k.a. The One)

https://github.com/florinbaciuu/lilygo-thmi-esp-idf-project.git


https://gitlab.com/florinbaciuu/lilygo-thmi-esp-idf-project.git


## Configurare Git globală (doar o dată pe sistem)
```
git config --global user.name "florinbaciuu"                    # Numele tău pentru toate commiturile
git config --global user.email "baciuaurelflorin@gmail.com"     # Emailul asociat contului GitHub
git config --global init.defaultBranch main
```

---

## Inițializarea unui proiect nou și conectarea la GitHub
```
git init                                                        # Inițializează repo local:
```
---

## Redenumește branch-ul în 'main':
```
git branch -M main
```
---

## Lucreaza cu fisiere
```
git add .                               # Add file contents to the index
git mv nume_fisier                      # Move or rename a file, a directory, or a symlink
git restore nume_fisier                 # Restore working tree files
git rm nume_fisier                      # Remove files from the working tree and from the index
```
---

## seteaza upstream
Atentie doar unul poate fii setat concomitent
```
git push --set-upstream origin main                             # Seteaza upstream github predefinit

# sau

git push --set-upstream gitlab main                             # Seteaza upstream gitlab
```
---

## Creează primul commit: si lucreaza cu commit
```
git commit -m "first commit"

git commit -m "first commit" --status
git commit --status
git commit --branch
git commit --all
```
---

## Ștergere remote
```
git remote remove origin                                          # Șterge remote-ul definit
git remote remove nume_remote
```
---

## Adaugă remote-ul : (GITHUB) and (GITLAB) in functie de necesitati
```
git remote add origin https://github.com/florinbaciuu/lilygo-thmi-esp-idf-project.git
git remote add github https://github.com/florinbaciuu/lilygo-thmi-esp-idf-project.git
git remote add gitlab https://gitlab.com/florinbaciuu/lilygo-thmi-esp-idf-project.git

```
---

## 🆕 Verifici remote-s
```
git remote -v
git remote
```
---

## Adăugare submodul Git (ex: o librărie în lib/) / Verificare submodule existente /
## Adăugare, verificare și ștergere submodul Git (ex: o librărie în lib/)

```
git submodule add https://github.com/florinbaciuu/ESP32_Resource_Monitor.git lib/ESP32_Resource_Monitor
git submodule status                  # Afișează commiturile și starea submodulelor
git config -f .gitmodules --list     # Verifică ce submodule sunt înregistrate oficial
cat .gitmodules                      # Afișează configurația direct
```
---

### ❌ Ștergere completă a unui submodul
```
git submodule deinit -f lib/ESP32_Resource_Monitor
git rm -f lib/ESP32_Resource_Monitor
git rm -f .git/modules/lib/ESP32_Resource_Monitor
rm -rf .git/modules/lib/ESP32_Resource_Monitor
git commit -m "Șters submodulul ESP32_Resource_Monitor"
```
---

## 🚀 Push către GitHub
```
git push -u origin main                                           # Push initial La primul push dintr-un proiect nou:
git push -u github main
git push -u gitlab main

git push --set-upstream origin main                               # Push initial La primul push dintr-un proiect nou:
git push --set-upstream github main
git push --set-upstream gitlab main

git branch -vv							                          # Verifica ce upstream e
git push 							                              # Dupa ce ai pus "git push -u origin main"

git push --force origin main                                      # Push cu forțare (atenție!)
git push --force github main
git push --force gitlab main

git push origin main
git push github main
git push gitlab main

git push --all                                                    # Dar fii atent --all inseamna toate branch-urile nu toti remote-ii

git fetch                                                         # Doar aduce modificările remote, nu le aplică local
git pull                                                          # Fetch + Merge (sau Rebase) = trage și le bagă direct în branchul tău
git clone                                                         # Fetch complet + creează repo nou local
git push                                                          # Trimite schimbările tale către remote
```
---

## Clonare cu tot cu submodule

```
git clone --recurse-submodules https://github.com/florinbaciuu/lilygo-thmi-esp-idf-project
git submodule status
git submodule update --init --recursive
```
---

##  Reparare submodul – commit lipsă / detached HEAD

##  Varianta 1: Forțezi commit valid
```
cd lib/ESP32_Resource_Monitor
git fetch
git checkout main
cd ../..
git add lib/ESP32_Resource_Monitor
git commit -m "Resetat submodulul la commit valid"
git push
```

## Varianta 2: Ștergi submodulul și îl adaugi curat
```
git submodule deinit -f lib/ESP32_Resource_Monitor
git rm -f lib/ESP32_Resource_Monitor
rm .git/modules/lib/ESP32_Resource_Monitor
git commit -m "Șters submodulul buclucaș"
```

Apoi:
```
git submodule add https://github.com/florinbaciuu/ESP32_Resource_Monitor.git lib/ESP32_Resource_Monitor
git commit -m "Adăugat din nou submodulul"
git push
```

## Sterge fisier din cache dupa introducerea in .gitignore
```
git rm -r --cached .devcontainer
git rm -r --cached .vscode
git rm -r --cached build

```

## Branch
```
git branch                                              # View local
git branch -r                                           # View remote
git branch -a                                           # View all (local and remote)
git branch -v                                           # View the branches

git branch -d nume_branch                               # Delete the branch work only if the branch fusiioned with main
git branch -D nume_branch                               # Delete the branch forced

git push origin --delete nume_branch                    # Sterge un branch de pe remote
git push github --delete nume_branch 
git push gitlab --delete nume_branch 

```

