    ############################################################################
    #                                                                          #
    #                        _                ____    _____                    #
    #                       (_)              / __ \  / ____|                   #
    #                        _   __ _   ___ | |  | || (___                     #
    #                       | | / _` | / _ \| |  | | \___ \                    #
    #                       | || (_| ||  __/| |__| | ____) |                   #
    #                       | | \__,_| \___| \____/ |_____/                    #
    #                      _/ |                                                #
    #                     |__/                                                 #
    #              _____   ______            _____   __  __  ______            #
    #             |  __ \ |  ____|    /\    |  __ \ |  \/  ||  ____|           #
    #             | |__) || |__      /  \   | |  | || \  / || |__              #
    #             |  _  / |  __|    / /\ \  | |  | || |\/| ||  __|             #
    #             | | \ \ | |____  / ____ \ | |__| || |  | || |____            #
    #             |_|  \_\|______|/_/    \_\|_____/ |_|  |_||______|           #   
    #                                                                          #  
    #                                                                          #
    #                                                                          #
    ############################################################################

### AUTORI
__Matteo Del Vecchio  
Simone Preite__  

### DECOMPRESSIONE DEL PROGETTO
Eseguire il seguente comando nella directory dove è contenuto l'archivio:  
                                                        
	$ cd /directory/in_cui_si_trova_archivio
	$ tar -xvf jaeOS.tar.gz

### COMPILAZIONE E PULIZIA
Spostarsi della directory ottenuta dall'estrazione dell'archivio e digitare il comando:

	$ make

per ottenere la compilazione di tutti gli eseguibili necessari, che saranno:

- jaeOS.elf
- jaeOS.elf.core.uarm
- jaeOS.elf.stab.uarm

i quali compariranno nella cartella `compiled/kernel`.  
E' possibile anche compilare solo __phase 0__ e __phase 1__ passando come argomenti `phase0` o `phase1`, ad esempio:

	make phase1

Inoltre, il make file offre la possibilità di eliminare i file creati attraverso le opzioni:

	make cleanall (che elimina anche gli altri file generati per uarm)
	make clean_X  (elimina solo gli eseguibili della fase in questione)

dove ad __X__ va sostituito `p0`, `p1`, `p2`, in base ai file oggetto ed eseguibili che si vogliono eliminare

### TEST P2TEST
Per verificare il `p2test` sulla macchina, aprimo un terminale ed eseguiamo:

	$ uarm

che avvierà la macchina (emulatore) spenta. Clicchiamo sul pulsante delle __impostazioni__ e nella sezione __Boot__ cerchiamo, attraverso *browse*, il file `jaeOS.elf.core.uarm`. Successivamente nella stessa finestra alla sezione `Symbol Table`, cerchiamo, attraverso *browse*, il file `jaeOS.elf.core.uarm` (che si troveranno nella directory `compiled/kernel` generata in fase di compilazione).  
Accendiamo l'emulatore attraverso il tasto __power__ e apriamo un terminale da __terminals__.  
Clicchiamo sul tasto __play__ fino a esecuzione completata per verificare l'output.

### INFO E LINK UTILI
Link per il download e informazioni utili su uARM: [http://mellotanica.github.io/uARM/](http://mellotanica.github.io/uARM/)  
Michael Goldweber, Renzo Davoli *µMPS2 Principles of Operation*. Lulu Books. 2011: [http://www.cs.unibo.it/~renzo/so/princOfOperations.pdf](http://www.cs.unibo.it/~renzo/so/princOfOperations.pdf)  
Michael Goldweber, Renzo Davoli *Student Guide to the Kaya Operating System*. Lulu Books. 2009: [http://www.cs.unibo.it/~renzo/so/Kaya.pdf](http://www.cs.unibo.it/~renzo/so/Kaya.pdf)  
Directory contenente le info utili e le specifiche per lo svolgimento del progetto jaeOS per l'anno accademico 2015/2016: [http://www.cs.unibo.it/~renzo/so/jaeos16/](http://www.cs.unibo.it/~renzo/so/jaeos16/)
