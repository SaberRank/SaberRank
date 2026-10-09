'use client';

import type { SubmitEvent } from 'react';
import { useState } from 'react';

import { useMutation } from '@tanstack/react-query';
import { useRouter } from '@tanstack/react-router';
import { CircleCheck, Loader2, TriangleAlert, UserRoundPlus } from 'lucide-react';
import { useTranslations } from 'use-intl';

import { Alert, AlertDescription, AlertTitle } from '@/components/ui/alert';
import { Button } from '@/components/ui/button';
import { Input } from '@/components/ui/input';
import { Label } from '@/components/ui/label';
import { completeSignup } from '@/modules/auth/actions/credentials';
import { useAuth } from '@/modules/auth';
import { unwrapAction } from '@/shared/result/action';

export function SignupForm({ redirectTo, onSignInSelect }: { redirectTo: string; onSignInSelect: () => void }) {
   const t = useTranslations();
   const router = useRouter();
   const { refreshAuth } = useAuth();
   const [email, setEmail] = useState('');
   const [displayName, setDisplayName] = useState('');
   const [password, setPassword] = useState('');
   const [feedback, setFeedback] = useState<{ ok: boolean; message: string } | null>(null);

   const mutation = useMutation({
      mutationFn: async () => unwrapAction(await completeSignup({ email: email.trim(), challengeId: 'direct', code: '000000', password, displayName: displayName.trim() })),
      onMutate: () => setFeedback(null),
      onSuccess: async (value) => {
         if (value.status === 'authenticated') {
            await refreshAuth();
            await router.navigate({ href: redirectTo, replace: true });
         }
      },
      onError: (error) => setFeedback({ ok: false, message: error instanceof Error ? error.message : t('login.signup.failedToast') })
   });

   function submit(event: SubmitEvent<HTMLFormElement>) {
      event.preventDefault();
      mutation.mutate();
   }

   return (
      <div className="flex w-full max-w-sm flex-col gap-4 text-left">
         <Alert variant="info">
            <UserRoundPlus aria-hidden />
            <AlertTitle>{t('login.signup.newPlayerLead')}</AlertTitle>
            <AlertDescription>{t('login.signup.directDescription')}</AlertDescription>
         </Alert>

         {feedback && (
            <Alert variant={feedback.ok ? 'default' : 'destructive'}>
               {feedback.ok ? <CircleCheck aria-hidden /> : <TriangleAlert aria-hidden />}
               <AlertTitle>{feedback.ok ? t('login.signup.sentToast') : t('login.signup.failedToast')}</AlertTitle>
               <AlertDescription>{feedback.message}</AlertDescription>
            </Alert>
         )}

         <form className="flex flex-col gap-3" onSubmit={submit}>
            <div className="flex flex-col gap-2">
               <Label htmlFor="signup-email">{t('login.email.emailLabel')}</Label>
               <Input id="signup-email" type="email" value={email} autoComplete="email" onChange={(e) => setEmail(e.target.value)} disabled={mutation.isPending} />
            </div>
            <div className="flex flex-col gap-2">
               <Label htmlFor="signup-name">{t('login.signup.displayNameLabel')}</Label>
               <Input id="signup-name" value={displayName} autoComplete="nickname" onChange={(e) => setDisplayName(e.target.value)} disabled={mutation.isPending} />
               <p className="text-muted-foreground text-xs">{t('login.signup.displayNameHelp')}</p>
            </div>
            <div className="flex flex-col gap-2">
               <Label htmlFor="signup-password">{t('login.password.passwordLabel')}</Label>
               <Input id="signup-password" type="password" value={password} autoComplete="new-password" onChange={(e) => setPassword(e.target.value)} disabled={mutation.isPending} />
               <p className="text-muted-foreground text-xs">{t('login.password.passwordHelp')}</p>
            </div>
            <Button type="submit" disabled={!email || displayName.trim().length < 2 || password.length < 10 || mutation.isPending} className="cursor-pointer">
               {mutation.isPending ? <Loader2 data-icon="inline-start" className="animate-spin" /> : <UserRoundPlus data-icon="inline-start" />}
               {t('login.signup.submit')}
            </Button>
         </form>

         <button type="button" onClick={onSignInSelect} className="text-muted-foreground hover:text-foreground cursor-pointer text-sm underline underline-offset-2">
            {t('login.signup.signInExisting')}
         </button>
      </div>
   );
}
