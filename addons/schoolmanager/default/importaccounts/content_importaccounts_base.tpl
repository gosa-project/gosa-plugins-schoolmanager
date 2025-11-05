{include file="`$phases_path`../common/fontawesome.tpl" inline}

<div class="card-content-scroll">
<div class="row">
    <div class="col s12">
        <div class="section">
            <p>
                {if $import_account_type == "studentsandparents"}
                    {t escape=no}
                        With GOsa2 School Manager's <b>Import Students and Parents Module</b>
                        you can import student user accounts, parent accounts, class groups and
                        course groups from a single CSV file containing one user's account
                        information per line.
                    {/t}
                {elseif $import_account_type == "teachers"}
                    {t escape=no}
                        With GOsa2 School Manager's <b>Import Teachers Module</b> you can import
                        teacher user accounts, class groups, course groups and subject groups
                        from a single CSV file containing one user's account information per
                        line.
                    {/t}
                {elseif $import_account_type == "studentsonly"}
                    {t escape=no}
                        With GOsa2 School Manager's <b>Import Students (only) Module</b>
                        you can import student user accounts, class groups and course groups from
                        a single CSV file containing one user's account information per line.
                    {/t}
                {/if}
            </p>
        <div>
        <hr class="divider">

        {* PHASE 1: *}
        {if $file_uploaded != true}
            {$phase_path="`$phases_path`content_importaccounts_phase1.tpl"}
            {include file="$phase_path" inline}

        {* PHASE 2: *}
        {elseif $import_configured != true}
            {$phase_path="`$phases_path`content_importaccounts_phase2.tpl"}
            {include file="$phase_path" inline}

        {* PHASE 3: *}
        {elseif $accounts_reviewed != true}
            {$phase_path="`$phases_path`content_importaccounts_phase3.tpl"}
            {include file="$phase_path" inline}

        {* PHASE 4: *}
        {elseif $accounts_imported != true}
            {$phase_path="`$phases_path`content_importaccounts_phase4.tpl"}
            {include file="$phase_path" inline}

        {* PHASE 5: *}
        {elseif $groups_reviewed != true}
            {$phase_path="`$phases_path`content_importaccounts_phase5.tpl"}
            {include file="$phase_path" inline}

        {* PHASE 6: *}
        {elseif $groups_imported != true}
            {$phase_path="`$phases_path`content_importaccounts_phase6.tpl"}
            {include file="$phase_path" inline}

        {* PHASE 7: *}
        {elseif $accounts_groupmembers_reviewed != true}
            {$phase_path="`$phases_path`content_importaccounts_phase7.tpl"}
            {include file="$phase_path" inline}

        {* PHASE 8: *}
        {elseif $accounts_groupmembers_updated != true}
            {$phase_path="`$phases_path`content_importaccounts_phase8.tpl"}
            {include file="$phase_path" inline}

        {* PHASE 9: *}
        {elseif $cleanup_completed != TRUE}
            {$phase_path="`$phases_path`content_importaccounts_phase9.tpl"}
            {include file="$phase_path" inline}

        {* PHASE 10: *}
        {else}
            {$phase_path="`$phases_path`content_importaccounts_phase10.tpl"}
            {include file="$phase_path" inline}

        {/if}

        {* SUBMIT: Different buttons depending on the above phases *}
        <div class="card-action">
            {if $file_uploaded == TRUE && $cleanup_completed != TRUE}
                <button class="btn-small" type="submit" name="cancel_import" style="margin-left: 0; margin-right: 20px;">{t}Cancel Import{/t}</button>
            {/if}

            {if $file_uploaded != TRUE}
                <span style="margin-left: 40px;">{t}Continue here, when ready{/t}:</span>
                <button class="btn-small primary" type='submit' name='file_uploaded'>
                    {t 1="2/10"}Set import configuration option (Step %1){/t}
                </button>

            {elseif $import_configured != TRUE}
                <button class="btn-small primary" type="submit" name="import_configured">
                    {t 1="3/10"}Review user account objects before LDAP import (Step %1){/t}
                </button>

            {elseif $accounts_reviewed != TRUE}
                {t}Continue here, when ready (this can take a while...){/t}:
                <button class="btn-small primary" type="submit" name="accounts_reviewed">
                    {t 1="4/10"}Import user account objects into LDAP (Step %1){/t}
                </button>

            {elseif $accounts_imported != TRUE}
                {t}Continue here, when ready{/t}:
                <button class="btn-small primary" type="submit" name="accounts_imported">
                    {t 1="5/10"}Review group objects (Step %1){/t}
                </button>

            {elseif $groups_reviewed != TRUE}
                {t}Continue here, when ready (this can take a while...){/t}:
                <button class="btn-small primary" type="submit" name="groups_reviewed">
                    {t 1="6/10"}Import group objects into LDAP (Step %1){/t}
                </button>

            {elseif $groups_imported != TRUE}
                {t}Continue here, when ready (this can take a while...){/t}:
                <button class="btn-small primary" type="submit" name="groups_imported">
                    {t 1="7/10"}Review accounts' group memberships before LDAP import (Step %1){/t}
                </button>

            {elseif $accounts_groupmembers_reviewed != TRUE}
                {t}Continue here, when ready (this can take a while...){/t}:
                <button class="btn-small primary" type="submit" name="accounts_groupmembers_reviewed">
                    {t 1="8/10"}Update accounts' group memberships in LDAP (Step %1){/t}
                </button>

            {elseif $accounts_groupmembers_updated != TRUE}
                {t}Continue here, when ready{/t}:
                <button class="btn-small primary" type="submit" name="accounts_groupmembers_updated">
                    {t 1="9/10"}Some post-import clean-ups (Step %1){/t}
                </button>

            {elseif $cleanup_completed != TRUE}
                {t}Continue here, when ready{/t}:
                <button class="btn-small primary" type="submit" name="cleanup_completed">
                    {t 1="10/10"}Finish LDAP import (Step %1){/t}
                </button>

            {/if}
        </div>
    </div>
</div>
</div>
