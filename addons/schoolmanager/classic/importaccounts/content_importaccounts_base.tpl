{include file="`$phases_path`../common/fontawesome.tpl" inline}

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

<hr>
{* PHASE 1: *}
{if $file_uploaded != true}
    {$phase_path="`$phases_path`content_importaccounts_phase1.tpl"}
    {include file="$phase_path" inline}

{* PHASE 2: *}
{elseif $import_configured != true}
    {$phase_path="`$phases_path`content_importaccounts_phase2.tpl"}
    {include file="$phase_path" inline}

{* PHASE 3: *}
{elseif $data_sorted != true}
    {$phase_path="`$phases_path`content_importaccounts_phase3.tpl"}
    {include file="$phase_path" inline}

{* PHASE 4: *}
{elseif $accounts_reviewed != true}
    {$phase_path="`$phases_path`content_importaccounts_phase4.tpl"}
    {include file="$phase_path" inline}

{* PHASE 5: *}
{elseif $accounts_imported != true}
    {$phase_path="`$phases_path`content_importaccounts_phase5.tpl"}
    {include file="$phase_path" inline}

{* PHASE 6: *}
{elseif $groups_reviewed != true}
    {$phase_path="`$phases_path`content_importaccounts_phase6.tpl"}
    {include file="$phase_path" inline}

{* PHASE 7: *}
{elseif $groups_imported != true}
    {$phase_path="`$phases_path`content_importaccounts_phase7.tpl"}
    {include file="$phase_path" inline}

{* PHASE 8: *}
{elseif $accounts_groupmembers_reviewed != true}
    {$phase_path="`$phases_path`content_importaccounts_phase8.tpl"}
    {include file="$phase_path" inline}

{* PHASE 9: *}
{elseif $accounts_groupmembers_updated != true}
    {$phase_path="`$phases_path`content_importaccounts_phase9.tpl"}
    {include file="$phase_path" inline}

{* PHASE 10: *}
{elseif $cleanup_completed != TRUE}
    {$phase_path="`$phases_path`content_importaccounts_phase10.tpl"}
    {include file="$phase_path" inline}

{* PHASE 11: *}
{else}
    {$phase_path="`$phases_path`content_importaccounts_phase11.tpl"}
    {include file="$phase_path" inline}

{/if}

<hr>
{* SUBMIT: Different buttons depending on the above phases *}
<div class="plugin-actions">

    {if $file_uploaded != TRUE}
        {t}Continue here, when ready{/t}:
        <button type='submit' name='file_uploaded'>
            {t 1="2/11"}Set import configuration option (Step %1){/t}
        </button>

        <p style="float:left; margin: 0px; padding: 0px;">
            <button type='submit' name='file_uploaded_do_quickimport'>
                {t escape='no'}Do <b>Quick-Import</b>{/t}
            </button>
        </p>

    {elseif $import_configured != TRUE}
        {t}Continue here, when ready{/t}:
        <button type="submit" name="import_configured">
            {t 1="3/11"}Check and Sort CSV Data(Step %1){/t}
        </button>

    {elseif $data_sorted != TRUE}
        {t}Continue here, when ready{/t}:
        <button type="submit" name="data_sorted">
            {t 1="4/11"}Review user account objects before LDAP import (Step %1){/t}
        </button>

    {elseif $accounts_reviewed != TRUE}
        {t}Continue here, when ready (this can take a while...){/t}:
        <button type="submit" name="accounts_reviewed">
            {t 1="5/11"}Import user account objects into LDAP (Step %1){/t}
        </button>

    {elseif $accounts_imported != TRUE}
        {t}Continue here, when ready{/t}:
        <button type="submit" name="accounts_imported">
            {t 1="6/11"}Review group objects (Step %1){/t}
        </button>

    {elseif $groups_reviewed != TRUE}
        {t}Continue here, when ready (this can take a while...){/t}:
        <button type="submit" name="groups_reviewed">
            {t 1="7/11"}Import group objects into LDAP (Step %1){/t}
        </button>

    {elseif $groups_imported != TRUE}
        {t}Continue here, when ready (this can take a while...){/t}:
        <button type="submit" name="groups_imported">
            {t 1="8/11"}Review accounts' group memberships before LDAP import (Step %1){/t}
        </button>

    {elseif $accounts_groupmembers_reviewed != TRUE}
        {t}Continue here, when ready (this can take a while...){/t}:
        <button type="submit" name="accounts_groupmembers_reviewed">
            {t 1="9/11"}Update accounts' group memberships in LDAP (Step %1){/t}
        </button>

    {elseif $accounts_groupmembers_updated != TRUE}
        {t}Continue here, when ready{/t}:
        <button type="submit" name="accounts_groupmembers_updated">
            {t 1="10/11"}Some post-import clean-ups (Step %1){/t}
        </button>

    {elseif $cleanup_completed != TRUE}
        {t}Continue here, when ready{/t}:
        <button type="submit" name="cleanup_completed">
            {t 1="11/11"}Finish LDAP import (Step %1){/t}
        </button>

    {/if}

    {if $file_uploaded == TRUE && $cleanup_completed != TRUE}
        <button type="submit" name="cancel_import">{t}Cancel Import{/t}</button>
    {/if}
</div>
