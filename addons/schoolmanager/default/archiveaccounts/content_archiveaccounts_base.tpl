{include file="`$phases_path`../common/fontawesome.tpl" inline}

<div class="card-content-scroll">
<div class="row">
    <div class="col s12">
        <p>
            {t escape=no}With GOsa2 School Manager's <b>Archive Accounts Module</b> you can archive student user accounts, parent accounts, teacher from multiple CSV files.{/t}
        </p>
        <hr class="divider">

        <!--Introduction-->
        <!-- PHASE 0: Choose archive type -->
        {if $chosen_archivetype != TRUE}

            {$phase_path="`$phases_path`content_archiveaccounts_phase0.tpl"}
            {include file="$phase_path" inline}

        <!-- PHASE 1: Upload CSV File-->
        {elseif $file_uploaded != TRUE}

            {$phase_path="`$phases_path`content_archiveaccounts_phase1.tpl"}
            {include file="$phase_path" inline}

        <!-- PHASE 2: Configuring archive options -->
        {elseif $archive_configured != TRUE}

            {$phase_path="`$phases_path`content_archiveaccounts_phase2.tpl"}
            {include file="$phase_path" inline}

        <!-- PHASE 3: CSV data sorting -->
        {elseif $data_sorted != TRUE}

            {$phase_path="`$phases_path`content_archiveaccounts_phase3.tpl"}
            {include file="$phase_path" inline}

        <!-- PHASE 4: Reviewing and confirming accounts before archiving -->
        {elseif $accounts_reviewed != TRUE}

            {$phase_path="`$phases_path`content_archiveaccounts_phase4.tpl"}
            {include file="$phase_path" inline}

        <!-- PHASE 5: Confirming archived user accounts -->
        {elseif $accounts_archived != TRUE}

            {$phase_path="`$phases_path`content_archiveaccounts_phase5.tpl"}
            {include file="$phase_path" inline}

        <!-- PHASE 6: Moving primary groups -->
        {elseif $primgroups_moved != TRUE}

            {$phase_path="`$phases_path`content_archiveaccounts_phase6.tpl"}
            {include file="$phase_path" inline}

        {/if}


        <!-- SUBMIT: Different buttons depending on the above phases -->

        <div class="card-action">
            {if $file_uploaded == TRUE && $primgroups_moved != TRUE}
                <button class="btn-small" type="submit" name="cancel_archiving" style="margin-left: 0; margin-right: 20px;">{t}Cancel archiving{/t}</button>
            {/if}

            {if $chosen_archivetype != TRUE}
            {t}Continue here, when ready{/t}: <button class="btn-small primary" type='submit' name='chosen_archivetype'>{t 1="2/11"}Upload CSV File (Step %1){/t}</button>

            {elseif $file_uploaded != TRUE}
            {t}Continue here, when ready{/t}: <button class="btn-small primary" type='submit' name='file_uploaded'>{t 1="3/11"}Set archive configuration option (Step %1){/t}</button>

            {elseif $archive_configured != TRUE}
            {t}Continue here, when ready{/t}: <button class="btn-small primary" type="submit" name="archive_configured">{t 1="4/11"}Check and Sort CSV Data (Step %1){/t}</button>

            {elseif $data_sorted != TRUE}
            {t}Continue here, when ready{/t}: <button class="btn-small primary" type="submit" name="data_sorted">{t 1="5/11"}Review Accounts (Step %1){/t}</button>

            {elseif $accounts_reviewed != TRUE}
            {t}Continue here, when ready{/t}: <button class="btn-small primary" type="submit" name="accounts_reviewed">{t 1="6/11"}Archive Accounts (Step %1){/t}</button>

            {elseif $accounts_archived != TRUE}
            {t}Continue here, when ready{/t}: <button class="btn-small primary" type="submit" name="accounts_archived">{t 1="7/11"}Move primary group (Step %1){/t}</button>

            {elseif $primgroups_moved != TRUE}
            <!-- {t}Continue here, when ready{/t}: <button class="btn-small primary" type="submit" name="primgroups_moved">{t 1="8/11"}stuff (Step %1){/t}</button> -->

            {/if}
        </div>
    </div>
</div>
</div>
