<input type="hidden" name="phase_05">

<br><h3>{t 1="6/11"}STEP %1: Confirm archived user accounts{/t}</h3>

{foreach from=$account item=row key=key}
    <table summary="{t 1=$account->secondName 2=$account->givenName}User account: %1, %2{/t}" cellspacing="1" border=0 cellpadding="4" bgcolor="#FEFEFE">
        <tr>
    {if isset($account->guardians)}
            <td bgcolor="#BBBBBB" colspan={2 + count($account->guardians)}>
    {else}
            <td bgcolor="#BBBBBB" colspan="2">
    {/if}
    {t 1=$account->secondName 2=$account->givenName}User account: %1, %2{/t}
            </td>
        </tr>
    {if isset($account->guardians)}
        <tr>
            <td>
                <table summary="{t 1=$account->secondName 2=$account->givenName}Account group for %1, %2{/t}">
    {/if}
    {foreach from=$account[$key]['main_account'] item=value key=property}
                    <tr>
                        <td bgcolor="#EEEEEE">
                            <b>{$property}:</b>
                        </td>
                        <td bgcolor="#F8F8F8">
        {if $property != "userPassword" || $account[$key]['main_account'][$property][0]===""}
                                {$account[$key]['main_account'][$property][0]}
        {elseif strpos($account->status,"exists")!==FALSE }
                            {t}<keep>{/t}
        {elseif strpos($account->status,"data-incomplete")!==FALSE }
        {else}
                            ************
        {/if}
                        </td>
                    </tr>
    {/foreach}
    </table>
    <br>
{/foreach}
