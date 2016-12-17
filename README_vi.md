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

#####                           AUTORI                                 #####
#                                                                          #
#  Matteo         Del Vecchio   matteo.delvecchio6@studio.unibo.it         #
#  Simone 	  Preite 	simone.preite@studio.unibo.it              #
#                                                                          #
############################################################################                                                    
#####                     DECOMPRIMERE IL PROGETTO                     #####
#                                                                          #
#   - eseguire il seguente comando nella directory dove è contenuto        #
#     l'archivio:                                                          #
#                                                                          #
#     $ cd /directory/in_cui_si_trova_archivio                             #
#     $ tar -xvf jaeOS.tar.gz                                              #
#                                                                          #
############################################################################

#####                      COMPILAZIONE E PULIZIA                      #####
#                                                                          #
#   - spostarsi della directory ottenuta dall'estrazione                   #
#     dell'archivio e digitare il comando:                                 #
#                                                                          #
#     $ make                                                               #
#                                                                          #
#     per ottenere la compilazione di tutti gli eseguibili necessari       #
#     che saranno:                                                         #
#                                                                          #
#     jaeOS.elf jaeOS.elf.core.uarm jaeOS.elf.stab.uarm                    #
#                                                                          #
#     che compariranno nella cartella compiled/kernel                      #
#                                                                          #
#     è possibile anche compilare solo phase 0 e phase 1 passando come     #
#                                                                          #
#     argomenti phase0 o phase1, i.e. make phase1                          #
#                                                                          #
#   - il make file offre la possibilità di eliminare i file creati         #
#     attraverso le opzioni:                                               #
#                                                                          #
#     make clean_X  (elimina solo gli eseguibili della fase in questione)  #
#                                                                          #
#     dove ad X va sostituito p0, p1, p2 in base ai file oggetto ed        #
#                                                                          #
#     eseguibili che si vuole eliminare                                    #
#                                                                          #
#     make cleanall (che elimina anche gli altri file generati per uarm)   #
#                                                                          #
############################################################################

#####                          TEST P2TEST                             #####
#                                                                          #
# - per verificare il p1test sulla macchina aprimo un terminale ed         #
#   eseguiamo:                                                             #
#                                                                          #
#   $ uarm                                                                 #
#                                                                          #
#   che avvierà la macchina spenta.                                        #
#                                                                          #
#   clicchiamo sul pulsante delle impostazioni e nella sezione "Boot"      #
#   cerchiamo attraverso browse il file "jaeOS.elf.core.uarm".             #
#   successivamente nella stessa finestra nella sezione "Symbol Table"     #
#   cerchiamo attraverso browse il file "jaeOS.elf.core.uarm".             #
#   (che si troveranno nella directory compile/kernel creata in fase di    #
#   compilazione)                                                          #
#                                                                          #
#   accendiamo l'emulatore attraverso il tasto power e apriamo un terminale#
#   da terminals.                                                          #
#                                                                          #
#   cliccare sul tasto "play" fino a esecuzione completata per verificare  #
#   l'output.                                                              #
#                                                                          #
############################################################################

#####                        INFO E LINK UTILI                         #####
#                                                                          #
#	link per lo sca e informazioni utili su uARM:                      #
#		http://mellotanica.github.io/uARM/                         #
#                                                                          #
#	Michael Goldweber, Renzo Davoli µMPS2 Principles of Operation.     #
#	Lulu Books. 2011:                                                  #
#		http://www.cs.unibo.it/~renzo/so/princOfOperations.pdf     #
#                                                                          #
#	Michael Goldweber, Renzo Davoli Student Guide                      # 
#	to the Kaya Operating System. Lulu Books. 2009:                    #
#		http://www.cs.unibo.it/~renzo/so/Kaya.pdf                  #
#                                                                          #
#	directory contenente le info utili per lo svolgimento del progetto #
#	jaeOS per l'anno 2015/2016:                                        #
#		http://www.cs.unibo.it/~renzo/so/jaeos16/                  #
#                                                                          #
############################################################################



                                                                                                  
                                                                                                  
                                                                           
