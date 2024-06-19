{if $accounts_reviewed != TRUE}
    <input type="hidden" name="phase_04">
    <br><h3>{t 1="5/11"}STEP %1: Review user account objects before archiving{/t}</h3>
{/if}

{foreach from=$accounts item=account key=key}
    <table summary="{t 1=$account->secondName 2=$account->givenName}User account: %1, %2{/t}" cellspacing="1" border=0 cellpadding="4" bgcolor="#FEFEFE">
        <tr>
{if $account->status === "not-found"}
    {t 1=$account->secondName 2=$account->givenName}New user account: %1, %2{/t}
{else}
    {t 1=$account->secondName 2=$account->givenName}User account: %1, %2{/t}
{/if}
            </td>
        </tr>
    {if $accounts_reviewed && $accounts_imported && !in_array("ignore", $account->actions)}
        <tr><td colspan="2" style="height: 0.2em;"></td></tr>
        <tr>
            <td bgcolor="#BBBBBB">
            <b>{t}Groups{/t}:</b>
            </td>
            <td bgcolor="#EEEEEE">
            <b>{t}Group membership actions{/t}:</b>
            </td>
        </tr>
    {/if}

    </table>
    <br>
{/foreach}
