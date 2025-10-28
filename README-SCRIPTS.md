# 📜 Script di Pubblicazione - Guida Rapida

## 🎯 Modo Più Semplice (Raccomandato)

**Un solo comando per fare tutto:**

```bash
bash publish-to-github.sh
```

Questo script interattivo ti guiderà attraverso tutti i 3 passi automaticamente.

---

## 🔧 Modo Avanzato (Passo per Passo)

Se preferisci più controllo, esegui manualmente i 3 script:

### 1. Setup Repository
```bash
bash setup-repository.sh
```
Crea struttura, file e documentazione.

### 2. Verifica Repository
```bash
bash verify-repository.sh
```
Controlla che tutto sia pronto.

### 3. Pubblica su GitHub
```bash
bash git-publish.sh
```
Pubblica il codice su GitHub.

---

## 📋 Script Disponibili

| Script | Descrizione | Quando Usarlo |
|--------|-------------|---------------|
| `publish-to-github.sh` | 🎯 Script master che esegue tutto | **Prima pubblicazione (RACCOMANDATO)** |
| `setup-repository.sh` | Prepara struttura e file | Solo se vuoi controllo manuale |
| `verify-repository.sh` | Verifica completezza | Prima di pubblicare |
| `git-publish.sh` | Pubblica su GitHub | Quando tutto è pronto |

---

## ❓ FAQ

**Q: Quale script devo usare?**
A: Usa `publish-to-github.sh` - fa tutto automaticamente.

**Q: Ho già eseguito setup, posso saltare al passo 2?**
A: Sì! Esegui solo `verify-repository.sh` e poi `git-publish.sh`.

**Q: Posso fermarmi dopo un passo e continuare dopo?**
A: Sì! Gli script sono indipendenti.

**Q: Come testo solo la compilazione?**
A: Esegui `verify-repository.sh`.

**Q: Ho fatto un errore, posso ripartire?**
A: Sì! Gli script sono idempotenti (puoi rieseguirli).

---

## 🆘 In caso di problemi

1. Leggi PUBLISH.md per istruzioni dettagliate
2. Controlla i messaggi di errore degli script
3. Verifica i prerequisiti (git, esphome)
4. Chiedi aiuto su GitHub Discussions

---

## 📚 Documentazione Completa

Leggi **PUBLISH.md** per:
- Istruzioni dettagliate
- Troubleshooting
- Comandi git utili
- Post-pubblicazione

---

**Quick Start:**
```bash
bash publish-to-github.sh
```

That's it! 🚀
